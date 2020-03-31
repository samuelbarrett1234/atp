#include "KnowledgeKernel.h"
#include "StatementArray.h"
#include <boost/functional/hash.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/identity.hpp>
#include <functional>


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

	// successor rules of a single statement
	auto succ_statement = [this](const Statement& stmt)
		-> StatementArrayPtr
	{
		auto p_sub_results = std::make_shared<StatementArray>(
			stmt.get_substitutions(m_rules)
			);
		auto p_def_results = std::make_shared<StatementArray>(
			stmt.replace_free_with_def(m_id_to_arity)
			);
		auto p_free_results = std::make_shared<StatementArray>(
			stmt.replace_free_with_free()
			);

		return StatementArray::try_concat(
			*StatementArray::try_concat(
				*p_sub_results, *p_def_results
			), *p_free_results
		);
	};

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

	auto p_stmts = dynamic_cast<StatementArray*>(
		_p_stmts.get());

	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	const auto& stmts_arr = p_stmts->raw();

	std::vector<StatementArrayPtr> results;
	results.reserve(stmts_arr.size());

	std::transform(stmts_arr.begin(), stmts_arr.end(),
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

	const auto& arr = p_stmts->raw();

	// call .type_check on each statement
	return std::all_of(arr.begin(), arr.end(),
		boost::bind(&Statement::type_check, _1, boost::ref(*this)));
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

	// check that, at each step, the left or right hand side of the
	// premise matches the left or right hand side of the conclusion
	// (using the premise as the pattern and the conclusion as the
	// trial). I.e. there will be four checks per element per
	// equality rule.

	auto p_premise = dynamic_cast<StatementArray*>(
		_p_premise.get());
	auto p_concl = dynamic_cast<StatementArray*>(
		_p_concl.get());

	ATP_LOGIC_ASSERT(p_premise != nullptr);
	ATP_LOGIC_ASSERT(p_concl != nullptr);

	const auto& arr_premise = p_premise->raw();
	const auto& arr_concl = p_concl->raw();

	const size_t n = arr_premise.size();
	ATP_LOGIC_ASSERT(arr_concl.size() == n);

	std::vector<bool> follows_result;
	follows_result.reserve(n);

	// call conclusion_statement.follows_from(premise_statement)

	std::transform(boost::make_zip_iterator(
		boost::make_tuple(arr_premise.begin(), arr_concl.begin())
	), boost::make_zip_iterator(
		boost::make_tuple(arr_premise.end(), arr_concl.end())
	), std::back_inserter(follows_result),
		[](boost::tuple<const Statement&, const Statement&> p)
		{ return p.get<1>().follows_from(p.get<0>()); });

	return follows_result;
}


void KnowledgeKernel::define_eq_rule(Statement& rule)
{
	ATP_LOGIC_PRECOND(rule.check_kernel(this));
	ATP_LOGIC_PRECOND(rule.type_check(*this));

	// construct rule statement from syntax tree
	m_rules.emplace_back(std::move(rule));
}


std::list<size_t> KnowledgeKernel::get_symbol_id_catalogue() const
{
	std::list<size_t> symb_ids;

	std::transform(m_id_to_name.begin(),
		m_id_to_name.end(),
		std::back_inserter(symb_ids),
		[](boost::bimap<size_t, std::string>::value_type a) { return a.left; });

	return symb_ids;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


