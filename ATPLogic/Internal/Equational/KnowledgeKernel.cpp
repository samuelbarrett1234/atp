#include "KnowledgeKernel.h"
#include "StatementArray.h"
#include "Semantics.h"
#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/identity.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


size_t KnowledgeKernel::get_integrity_code() const
{
	// this is a temporary and a little bit hacky:
	// we compute the hash of the defined symbols (this is okay) but
	// the way we compute the hash of the user-defined equivalences
	// is a bit crap!

	size_t hash = boost::hash_range(
		m_name_to_arity.begin(), m_name_to_arity.end());
	
	hash += m_rules.size() * 31;  // this is the hacky, temporary bit

	return hash;
}


std::vector<StatementArrayPtr> KnowledgeKernel::succs(
	StatementArrayPtr _p_stmts) const
{
	ATP_LOGIC_PRECOND(valid(_p_stmts));  // expensive :(

	// The successor statements in equational logic are:
	// - Replacing either side of an equation with one of the given
	//   equality rules (just transitivity in disguise; if x=y is
	//   our statement, and we have a rule of the form y=z, then we
	//   can turn the statement into x=z.)
	// - Replacing a free variable with any of: a user-defined
	//   constant, a user-defined function whose arguments are new
	//   free variables, or a free variable which already exists
	//   within the statement.
	// For example, with the latter rule, if our statement is
	// "f(x)=g(x)" then we can obtain:
	// "f(e)=g(e)" for user defined "e", and "f(h(x))=g(h(x))" for
	// user defined "h", and instead if our statement is
	// "f(x, y)=g(h(x), y)" then "f(x, x)=g(h(x), x)".

	// successor rules of a single statement
	auto succ_statement = [this](const Statement& stmt)
		-> StatementArrayPtr
	{
		return StatementArray::try_concat(
			*StatementArray::try_concat(
				semantics::get_substitutions(stmt, m_rules),
				semantics::replace_free_with_def(stmt, m_id_to_arity)
			), semantics::replace_free_with_free(stmt)
		);
	};

	auto p_stmts = dynamic_cast<StatementArray*>(
		_p_stmts.get());

	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	std::vector<StatementArrayPtr> results;
	results.reserve(p_stmts->size());

	std::transform(p_stmts->begin(), p_stmts->end(),
		std::back_inserter(results), succ_statement);

	return results;
}


bool KnowledgeKernel::valid(
	StatementArrayPtr _p_stmts) const
{
	auto p_stmts = dynamic_cast<StatementArray*>(
		_p_stmts.get());

	if (p_stmts == nullptr)
		return false;

	// we will use a fold to check validity!

	// eq is valid iff both its sides are valid:
	auto eq_valid = boost::phoenix::arg_names::arg1
		&& boost::phoenix::arg_names::arg2;

	// can't get a free variable wrong:
	auto free_valid = boost::phoenix::val(true);

	auto const_valid = [this](size_t symb_id)
	{
		return id_is_defined(symb_id)
			&& symbol_arity_from_id(symb_id) == 0;
	};

	auto func_valid = [this](size_t symb_id,
			std::list<bool>::iterator child_begin,
			std::list<bool>::iterator child_end)
	{
		const size_t implied_arity = std::distance(child_begin,
			child_end);

		return id_is_defined(symb_id)
			&& symbol_arity_from_id(
				symb_id) == implied_arity
			&& std::all_of(child_begin, child_end,
				// use phoenix for an easy identity function
				boost::phoenix::arg_names::arg1);
	};

	auto stmt_valid = [&eq_valid, &free_valid, &const_valid,
		&func_valid](const Statement& stmt)
	{
		return stmt.fold<bool>(eq_valid, free_valid, const_valid,
			func_valid);
	};

	return std::all_of(p_stmts->begin(), p_stmts->end(),
		stmt_valid);
}


std::vector<bool> KnowledgeKernel::follows(
	StatementArrayPtr _p_premise, StatementArrayPtr _p_concl) const
{
	ATP_LOGIC_PRECOND(_p_premise->size() == _p_concl->size());

#ifdef ATP_LOGIC_DEFENSIVE
	// these are quite expensive checks:
	ATP_LOGIC_PRECOND(valid(_p_premise));
	ATP_LOGIC_PRECOND(valid(_p_concl));
#endif

	auto p_premise = dynamic_cast<StatementArray*>(
		_p_premise.get());
	auto p_concl = dynamic_cast<StatementArray*>(
		_p_concl.get());

	ATP_LOGIC_ASSERT(p_premise != nullptr);
	ATP_LOGIC_ASSERT(p_concl != nullptr);

	std::vector<bool> follows_result;
	follows_result.reserve(p_premise->size());

	// call conclusion_statement.follows_from(premise_statement)

	std::transform(boost::make_zip_iterator(
		boost::make_tuple(p_premise->begin(), p_concl->begin())
	), boost::make_zip_iterator(
		boost::make_tuple(p_premise->end(), p_concl->end())
	), std::back_inserter(follows_result),
	[](boost::tuple<Statement, Statement> p)
	{ return semantics::follows_from(p.get<0>(), p.get<1>()); });

	return follows_result;
}


std::vector<StmtForm> KnowledgeKernel::get_form(
	StatementArrayPtr _p_stmts) const
{
	auto p_stmts = dynamic_cast<StatementArray*>(
		_p_stmts.get());

	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	std::vector<StmtForm> result;
	result.reserve(p_stmts->size());

	auto get_form = [this](const Statement& stmt) -> StmtForm
	{
		// a statement is trivially true if it is symmetric
		// about the equals sign (thus is true by reflexivity
		// of equality) or is a "rule" (which we take as an
		// axiom).
		// statements cannot be canonically false.

		if (semantics::true_by_reflexivity(stmt))
			return StmtForm::CANONICAL_TRUE;
		else if (is_a_rule(stmt))
			return StmtForm::CANONICAL_TRUE;
		else
			return StmtForm::NOT_CANONICAL;
	};

	std::transform(p_stmts->begin(), p_stmts->end(),
		std::back_inserter(result), get_form);

	return result;
}


void KnowledgeKernel::define_eq_rules(StatementArrayPtr _p_rules)
{
	ATP_LOGIC_PRECOND(valid(_p_rules));

	// try casting to equational::StatememtArray
	auto p_rules = dynamic_cast<const StatementArray*>(
		_p_rules.get());

	ATP_LOGIC_ASSERT(p_rules != nullptr);

	// construct rule statement from syntax tree
	m_rules.insert(m_rules.end(), p_rules->begin(),
		p_rules->end());
}


bool KnowledgeKernel::is_a_rule(const Statement& stmt) const
{
	return std::any_of(m_rules.begin(), m_rules.end(),
		boost::bind(&semantics::equivalent, boost::ref(stmt), _1));
}


std::list<size_t> KnowledgeKernel::get_symbol_id_catalogue() const
{
	std::list<size_t> symb_ids;

	std::transform(m_id_to_name.begin(),
		m_id_to_name.end(),
		std::back_inserter(symb_ids),
		[](boost::bimap<size_t, std::string>::value_type a)
		{ return a.left; });

	return symb_ids;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


