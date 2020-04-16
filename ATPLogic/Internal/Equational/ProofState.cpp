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
	if (m_next_begin_iter == nullptr)
	{
		// compute it because we don't have a cache anyways
		return compute_begin();
	}
	else
	{
		// we want to lose ownership of m_next_begin_iter because
		// the user will generally modify the object returned
		// from this function, and that is entirely designed
		// behaviour - hence we cannot hold onto the cache here.

		auto ptr = std::move(m_next_begin_iter);

		// just to make absolutely sure that std::move will erase
		// this from our object
		ATP_LOGIC_ASSERT(m_next_begin_iter == nullptr);

		return ptr;
	}
}


ProofCompletionState ProofState::completion_state() const
{
	if (!m_comp_state.has_value())
	{
		if (m_ker.is_trivial(m_current))
			return ProofCompletionState::PROVEN;
		else
		{
			if (m_next_begin_iter == nullptr)
			{
				m_next_begin_iter = compute_begin();
			}

			if (m_next_begin_iter->valid())
				return ProofCompletionState::UNFINISHED;
			else
				// if begin iterator is invalid then we have no
				// successors
				return ProofCompletionState::NO_PROOF;
		}
	}

	return *m_comp_state;
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


PfStateSuccIterPtr ProofState::compute_begin() const
{
	// we need to increment the free variable IDs in `m_current` so
	// that they are guaranteed not to clash with any of the matching
	// rules in the knowledge kernel
	return SubExprMatchingIterator::construct(m_ctx, m_ker,
		m_target, m_current);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


