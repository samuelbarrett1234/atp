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
	ProofState(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		Statement target, Statement current);

	ProofState(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		Statement target);

	inline const IStatement& target_stmt() const override
	{
		return m_target;
	}

	PfStateSuccIterPtr succ_begin() const override;

	ProofCompletionState completion_state() const override;


	// not part of the IProofState interface:
	inline const Statement& forefront() const
	{
		return m_current;
	}

private:
	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	Statement m_target, m_current;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


