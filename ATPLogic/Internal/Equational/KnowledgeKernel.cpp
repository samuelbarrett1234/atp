/**

\file

\author Samuel Barrett

*/


#include "KnowledgeKernel.h"
#include <sstream>
#include <boost/functional/hash.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include "Semantics.h"
#include "ProofState.h"


namespace atp
{
namespace logic
{
namespace equational
{


KnowledgeKernel::KnowledgeKernel(const ModelContext& ctx) :
	m_context(ctx),
	m_next_thm_ref_id(1)
{ }


KnowledgeKernelPtr KnowledgeKernel::try_construct(
	const Language& lang, const ModelContext& ctx)
{
	auto p_ker = std::make_shared<KnowledgeKernel>(ctx);

	std::stringstream s;

	for (size_t i = 0; i < ctx.num_axioms(); ++i)
	{
		std::stringstream s;
		s << ctx.axiom_at(i) << std::endl;
	}

	p_ker->m_axioms = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	if (p_ker->m_axioms == nullptr)
		return KnowledgeKernelPtr();

	p_ker->rebuild_active_rules();

	return p_ker;
}


size_t KnowledgeKernel::get_integrity_code() const
{
	// it is important that we include the arity in this check
	// as well as the name

	const auto& id_to_arity_map = m_context.id_to_arity_map();

	size_t hash = boost::hash_range(
		id_to_arity_map.begin(), id_to_arity_map.end());
	
	// kind of hacky: convert all theorems to strings, then hash
	// those strings

	// warning: this is testing for identical rules, not equivalent
	// rules (which allow free variable permutations). Are we sure
	// this is what we want?

	std::vector<std::string> rules_str;
	rules_str.reserve(m_active_rules->size());
	std::transform(m_active_rules->begin(),
		m_active_rules->end(),
		std::back_inserter(rules_str),
		boost::bind(&Statement::to_str, _1));

	hash += boost::hash_range(rules_str.begin(),
		rules_str.end());

	return hash;
}


ProofStatePtr KnowledgeKernel::begin_proof_of(
	const IStatement& _stmt) const
{
	const Statement* p_stmt = dynamic_cast<const Statement*>(
		&_stmt);

	ATP_LOGIC_PRECOND(p_stmt != nullptr);
	ATP_LOGIC_PRECOND(type_check(m_context, *p_stmt));

	return std::make_shared<ProofState>(*this, *p_stmt);
}


bool KnowledgeKernel::is_trivial(
	const IStatement& _stmt) const
{
	const Statement* p_stmt = dynamic_cast<const Statement*>(
		&_stmt);

	ATP_LOGIC_PRECOND(p_stmt != nullptr);
	ATP_LOGIC_PRECOND(type_check(m_context, *p_stmt));

	// "implies" here is necessary for unidirectional truths,
	// "equivalent" is necessary for bidirectional truths,
	// "reflexivity" is for the last case of trivialities

	return std::any_of(m_active_rules->begin(),
		m_active_rules->end(),
		boost::bind(&semantics::implies, _1, boost::ref(*p_stmt))
	|| boost::bind(&semantics::equivalent, _1, boost::ref(*p_stmt)))
		|| semantics::true_by_reflexivity(*p_stmt);
}


size_t KnowledgeKernel::add_theorems(StatementArrayPtr p_thms)
{
	ATP_LOGIC_PRECOND(p_thms->size() > 0);
	ATP_LOGIC_PRECOND(p_thms != nullptr);
	ATP_LOGIC_ASSERT(m_theorems.find(m_next_thm_ref_id)
		== m_theorems.end());

#ifdef ATP_LOGIC_DEFENSIVE
	// only type-check the theorems in defensive mode because this is
	// slow
	for (size_t i = 0; i < p_thms->size(); ++i)
	{
		auto p_stmt = dynamic_cast<const Statement*>(&p_thms->at(i));

		ATP_LOGIC_PRECOND(p_stmt != nullptr);
		ATP_LOGIC_PRECOND(type_check(m_context,
			*p_stmt));
	}
#endif

	m_theorems[m_next_thm_ref_id] = p_thms;

	rebuild_active_rules();

	return m_next_thm_ref_id++;
}


void KnowledgeKernel::remove_theorems(size_t ref_id)
{
	auto iter = m_theorems.find(ref_id);

	ATP_LOGIC_PRECOND(iter != m_theorems.end());

	m_theorems.erase(iter);

	rebuild_active_rules();
}


bool KnowledgeKernel::type_check(const ModelContext& ctx,
	const Statement& stmt)
{
	// we will use a fold to check validity!

	// eq is valid iff both its sides are valid:
	auto eq_valid = boost::phoenix::arg_names::arg1
		&& boost::phoenix::arg_names::arg2;

	// can't get a free variable wrong:
	auto free_valid = boost::phoenix::val(true);

	auto const_valid = [&ctx](size_t symb_id)
	{
		return ctx.is_defined(symb_id)
			&& ctx.symbol_arity(symb_id) == 0;
	};

	auto func_valid = [&ctx](size_t symb_id,
		std::vector<bool>::iterator child_begin,
		std::vector<bool>::iterator child_end)
	{
		const size_t implied_arity = std::distance(child_begin,
			child_end);

		return ctx.is_defined(symb_id)
			&& ctx.symbol_arity(
				symb_id) == implied_arity
			&& std::all_of(child_begin, child_end,
				// use phoenix for an easy identity function
				boost::phoenix::arg_names::arg1);
	};

	return stmt.fold<bool>(eq_valid, free_valid, const_valid,
		func_valid);
}


void KnowledgeKernel::rebuild_active_rules()
{
	auto get_second = boost::bind(&std::pair<size_t,
		StatementArrayPtr>::second, _1);
	std::vector<StatementArrayPtr> thms(
		boost::make_transform_iterator(m_theorems.begin(),
			get_second), boost::make_transform_iterator(
				m_theorems.end(), get_second));

	// these shouldn't return nullptr
	_m_active_rules = StatementArray::try_concat(
		*m_axioms,
		*StatementArray::try_concat(
			thms));

	m_active_rules = dynamic_cast<const StatementArray*>(
		_m_active_rules.get());

	ATP_LOGIC_ASSERT(m_active_rules != nullptr);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


