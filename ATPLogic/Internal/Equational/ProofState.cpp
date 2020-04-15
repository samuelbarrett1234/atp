/**

\file

\author Samuel Barrett

*/


#include "ProofState.h"
#include "SubExprMatchingIterator.h"


namespace atp
{
namespace logic
{
namespace equational
{


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target, Statement current) :
	m_ctx(ctx), m_ker(ker), m_target(target),
	m_current(current)
{
	check_forefront_ids();
}


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target) :
	m_target(target),
	m_current(target),
	m_ker(ker), m_ctx(ctx)
{
	check_forefront_ids();
}


PfStateSuccIterPtr ProofState::succ_begin() const
{
	// we need to increment the free variable IDs in `m_current` so
	// that they are guaranteed not to clash with any of the matching
	// rules in the knowledge kernel
	return SubExprMatchingIterator::construct(m_ctx, m_ker,
		m_target, m_current);
}


ProofCompletionState ProofState::completion_state() const
{
	if (m_ker.is_trivial(m_current))
		return ProofCompletionState::PROVEN;
	else if (!succ_begin()->valid())
		return ProofCompletionState::NO_PROOF;
	else
		return ProofCompletionState::UNFINISHED;
}


void ProofState::check_forefront_ids()
{
	const auto ids = m_current.free_var_ids();
	if (std::any_of(ids.begin(), ids.end(),
		boost::bind(std::less_equal<size_t>(), _1,
			m_ker.get_rule_free_id_bound())))
	{
		m_current = m_current.increment_free_var_ids(
			m_ker.get_rule_free_id_bound() + 1);
	}
}

}  // namespace equational
}  // namespace logic
}  // namespace atp


