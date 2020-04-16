/**

\file

\author Samuel Barrett

*/


#include <boost/algorithm/string/join.hpp>
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
	m_ctx(ctx), m_ker(ker),
	m_proof_stack({ std::move(target), std::move(current) })
{
	check_forefront_ids();
}


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target) :
	m_proof_stack({ std::move(target) }),
	m_ker(ker), m_ctx(ctx)
{
	check_forefront_ids();
}


ProofState::ProofState(const ProofState& parent,
	Statement forefront) :
	m_ctx(parent.m_ctx), m_ker(parent.m_ker),

	// todo: do better than a copy here
	m_proof_stack(parent.m_proof_stack)
{
	m_proof_stack.emplace_back(std::move(forefront));

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
		if (m_ker.is_trivial(forefront()))
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


std::string ProofState::to_str() const
{
	std::list<std::string> as_strs;
	for (const auto& stmt : m_proof_stack)
		as_strs.push_back(stmt.to_str());

	return boost::algorithm::join(as_strs, "\n");
}


void ProofState::check_forefront_ids()
{
	const auto ids = forefront().free_var_ids();
	if (std::any_of(ids.begin(), ids.end(),
		boost::bind(std::less_equal<size_t>(), _1,
			m_ker.get_rule_free_id_bound())))
	{
		// modify forefront (which is exactly just the back element
		// of the stack)
		m_proof_stack.back() = forefront().increment_free_var_ids(
			m_ker.get_rule_free_id_bound() + 1);
	}
}


PfStateSuccIterPtr ProofState::compute_begin() const
{
	// we need to increment the free variable IDs in `m_current` so
	// that they are guaranteed not to clash with any of the matching
	// rules in the knowledge kernel
	return SubExprMatchingIterator::construct(m_ctx, m_ker,
		*this, forefront());
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


