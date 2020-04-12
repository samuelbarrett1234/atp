#pragma once


/**

\file

\author Samuel Barrett

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IProofState.h"
#include "Statement.h"
#include "KnowledgeKernel.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief This object enumerates successors of the equational proof
	states.
*/
class ATP_LOGIC_API PfStateSuccIterator :
	public ISuccessorIterator
{
public:
	PfStateSuccIterator(const KnowledgeKernel& ker,
		const Statement& current);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;

private:
	StatementArray m_succs;
	const KnowledgeKernel& m_ker;
	Statement m_stmt;
};


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
	ProofState(const KnowledgeKernel& ker,
		Statement target) :
		m_target(target),
		m_current(target),
		m_ker(ker)
	{ }

	inline const IStatement& target_stmt() const override
	{
		return m_target;
	}

	inline SuccIterPtr succ_begin() const override
	{
		return std::make_shared<PfStateSuccIterator>(m_current);
	}

	inline ProofCompletionState completion_state() const override
	{
		if (m_ker.is_trivial(m_current))
			return ProofCompletionState::PROVEN;
		else if (!succ_begin()->valid())
			return ProofCompletionState::NO_PROOF;
		else
			return ProofCompletionState::UNFINISHED;
	}

private:
	const KnowledgeKernel& m_ker;
	Statement m_target, m_current;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


