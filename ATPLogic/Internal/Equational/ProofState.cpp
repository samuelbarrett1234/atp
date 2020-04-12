/**

\file

\author Samuel Barrett

*/


#include "ProofState.h"
#include "Semantics.h"


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


ProofState::PfStateSuccIterator::PfStateSuccIterator(
	const ProofState& parent,
	const Statement& stmt,
	const StatementArray& succs) :
	m_parent(parent),
	m_stmt(stmt),
	m_succs(succs),
	m_index(0)
{ }


bool ProofState::PfStateSuccIterator::valid() const
{
	return (m_index < m_succs.size());
}


ProofStatePtr ProofState::PfStateSuccIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());

	return std::make_shared<ProofState>(
		m_parent.m_ctx, m_parent.m_ker, m_parent.m_target,
		m_succs.my_at(m_index));
}


void ProofState::PfStateSuccIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());

	++m_index;
}


PfStateSuccIterPtr ProofState::succ_begin() const
{
	if (m_succs == nullptr)
		m_succs = std::make_shared<StatementArray>(semantics::get_successors(
			m_ctx, m_current, m_ker.get_active_rules()));

	return std::make_shared<PfStateSuccIterator>(*this, m_current,
		dynamic_cast<const StatementArray&>(*m_succs));
}


ProofCompletionState ProofState::completion_state() const
{
	if (m_ker.is_trivial(m_current))
		return ProofCompletionState::PROVEN;
	else
	{
		if (m_succs == nullptr)
			m_succs = std::make_shared<StatementArray>(semantics::get_successors(
				m_ctx, m_current, m_ker.get_active_rules()));

		if (m_succs->size() == 0)
			return ProofCompletionState::NO_PROOF;
		else
			return ProofCompletionState::UNFINISHED;
	}
}

}  // namespace equational
}  // namespace logic
}  // namespace atp


