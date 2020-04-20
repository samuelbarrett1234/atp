/**

\file

\author Samuel Barrett

*/


#include <boost/algorithm/string/join.hpp>
#include "ProofState.h"
#include "SubExprMatchingIterator.h"
#include "NoRepeatIterator.h"


namespace atp
{
namespace logic
{
namespace equational
{


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target, Statement current,
	bool no_repeats, bool randomised) :
	m_ctx(ctx), m_ker(ker),
	m_proof(std::make_shared<StmtList>(std::move(target))),
	m_no_repeats(no_repeats), m_randomised(randomised)
{
	m_proof = std::make_shared<StmtList>(std::move(current),
		std::move(m_proof));
	check_forefront_ids();
}


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target,
	bool no_repeats, bool randomised) :
	m_proof(std::make_shared<StmtList>(std::move(target))),
	m_ker(ker), m_ctx(ctx),
	m_no_repeats(no_repeats), m_randomised(randomised)
{
	check_forefront_ids();
}


ProofState::ProofState(const ProofState& parent,
	Statement forefront) :
	m_ctx(parent.m_ctx), m_ker(parent.m_ker),
	m_proof(std::make_shared<StmtList>(std::move(forefront),
		parent.m_proof)),
	m_no_repeats(parent.m_no_repeats),
	m_randomised(parent.m_randomised)
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
	// compute and cache this value if we haven't computed it already
	if (!m_comp_state.has_value())
	{
		if (m_ker.is_trivial(forefront()))
			m_comp_state = ProofCompletionState::PROVEN;
		else
		{
			if (m_next_begin_iter == nullptr)
			{
				m_next_begin_iter = compute_begin();
			}

			if (m_next_begin_iter->valid())
				m_comp_state = ProofCompletionState::UNFINISHED;
			else
				// if begin iterator is invalid then we have no
				// successors
				m_comp_state = ProofCompletionState::NO_PROOF;
		}
	}

	return *m_comp_state;
}


std::string ProofState::to_str() const
{
	std::list<std::string> as_strs;

	auto p_list = m_proof.get();
	do
	{
		as_strs.push_front(p_list->head.to_str());
		p_list = p_list->tail.get();
	} while (p_list != nullptr);

	return boost::algorithm::join(as_strs, "\n");
}


void ProofState::check_forefront_ids()
{
	const auto ids = forefront().free_var_ids();
	if (std::any_of(ids.begin(), ids.end(),
		boost::bind(std::less_equal<size_t>(), _1,
			m_ker.get_rule_free_id_bound())))
	{
		// modify forefront (which is exactly just the head element).
		m_proof->head = forefront().increment_free_var_ids(
			m_ker.get_rule_free_id_bound() + 1);
	}
}


PfStateSuccIterPtr ProofState::compute_begin() const
{
	PfStateSuccIterPtr iter = SubExprMatchingIterator::construct(m_ctx, m_ker,
		*this, forefront(), m_randomised);

	// user has indicated they want a no-repeat iterator
	if (m_no_repeats)
	{
		iter = NoRepeatIterator::construct(*this, std::move(iter));
	}

	return iter;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


