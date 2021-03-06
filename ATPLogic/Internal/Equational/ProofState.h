#pragma once


/**

\file

\author Samuel Barrett

*/


#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IProofState.h"
#include "Statement.h"
#include "KnowledgeKernel.h"
#include "ModelContext.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief Proof states for equational logic

\details A proof state in equational logic is just a linked list of
	statements, where the head element is the target statement and
	the last element is the "forefront" of the proof. The proof is
	complete when the forefront statement is trivial.
*/
class ATP_LOGIC_API ProofState :
	public IProofState
{
public:
	/**
	\brief Linked list object for storing proofs that are potentially
		shared between proof state objects.
	*/
	struct ATP_LOGIC_API StmtList
	{
		StmtList() = default;
		StmtList(Statement&& head) :
			head(head)
		{ }
		StmtList(Statement&& head,
			std::shared_ptr<StmtList> tail) :
			head(head), tail(std::move(tail))
		{ }

		Statement head;
		std::shared_ptr<StmtList> tail;
	};

public:
	ProofState(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		Statement target, Statement current,
		bool no_repeats, bool randomised);

	ProofState(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		Statement target,
		bool no_repeats, bool randomised);

	ProofState(const ProofState& parent,
		Statement forefront);

	inline const IStatement& target_stmt() const override
	{
		auto p_list = m_proof.get();
		while (p_list->tail != nullptr)
			p_list = p_list->tail.get();
		return p_list->head;
	}

	PfStateSuccIterPtr succ_begin() const override;

	ProofCompletionState completion_state() const override;


	// not part of the IProofState interface:
	inline const Statement& forefront() const
	{
		return m_proof->head;
	}

	inline const StmtList* get_proof_list() const
	{
		return m_proof.get();
	}

	std::string to_str() const override;

	std::vector<size_t> get_usage(
		const StatementArrayPtr& p_stmts) const override;

private:
	/**
	\brief Check that the forefront statement's free variable IDs
		are out of the range used by the knowledge kernel (see
		the precondition for SubExprMatchingIterator)

	\see SubExprMatchingIterator
	*/
	void check_forefront_ids();

	/**
	\brief Helper function for creating a begin iterator (the
		difference with `succ_begin` is that this function does
		not attempt to use the cache.)
	*/
	PfStateSuccIterPtr compute_begin() const;

private:
	// flag for not repeating states in an iteration
	const bool m_no_repeats;
	// flag for enumerating successors in a random order
	const bool m_randomised;

	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	std::shared_ptr<StmtList> m_proof;

	// to save us computing the begin iterator several times, for
	// state checking and to return from `succ_begin`, we store
	// a copy here
	// if this is non-null then it has recently been constructed
	// and is owned only by this object
	// when `succ_begin` is called, if this is non-null then we
	// return it and call m_next_begin_iter.reset()
	mutable PfStateSuccIterPtr m_next_begin_iter;

	// to save us computing the begin iterator several times if
	// `completion_state` is called several times, cache the
	// result here
	// note that computing the completion state involves creating
	// a begin iterator which is stored in `m_next_begin_iter`.
	mutable boost::optional<ProofCompletionState> m_comp_state;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


