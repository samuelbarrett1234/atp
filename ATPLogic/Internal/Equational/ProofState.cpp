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


ProofState::PfStateSuccIterator::PfStateSuccIterator(
	const ProofState& parent,
	const KnowledgeKernel& ker,
	const Statement& stmt) :
	m_parent(parent),
	m_stmt(stmt),
	m_ker(ker),
	m_succs(semantics::get_successors(stmt,
		ker.get_active_rules())),
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
		m_ker, m_parent.m_target, m_succs.my_at(m_index));
}


void ProofState::PfStateSuccIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());

	++m_index;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


