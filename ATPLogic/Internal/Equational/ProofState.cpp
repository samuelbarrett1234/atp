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
{ }


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target) :
	m_target(target),
	m_current(target),
	m_ker(ker), m_ctx(ctx)
{ }


PfStateSuccIterPtr ProofState::succ_begin() const
{
	// we need to increment the free variable IDs in `m_current` so
	// that they are guaranteed not to clash with any of the matching
	// rules in the knowledge kernel
	return SubExprMatchingIterator::construct(m_ctx, m_ker,
		m_target, m_current.increment_free_var_ids(
			m_ker.get_rule_free_id_bound()));
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

}  // namespace equational
}  // namespace logic
}  // namespace atp


