#pragma once


/**
\file

\author Samuel Barrett

\brief IStmtSuccIterator implementation for equational logic

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStmtSuccIterator.h"
#include "Statement.h"
#include "KnowledgeKernel.h"
#include "ProofState.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\note Since most of this logic has already been implemented in the
	proof state iterators, we will just make this class act as an
	adapter for that interface
*/
class StmtSuccIterator :
	public IStmtSuccIterator
{
public:
	StmtSuccIterator(const Statement& my_root,
		const ModelContext& ctx,
		const KnowledgeKernel& ker);
	
	void advance() override;
	bool valid() const override;
	const IStatement& get() const override;
	StmtSuccIterPtr dive() const override;

private:
	Statement get_current() const;

private:
	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	ProofState m_pf_state;
	PfStateSuccIterPtr m_pf_st_iter;
	std::unique_ptr<Statement> m_current;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


