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


PfStateSuccIterator::PfStateSuccIterator(
	const KnowledgeKernel& ker,
	const Statement& stmt) :
	m_stmt(stmt),
	m_ker(ker),
	m_succs(semantics::get_successors(stmt,
		ker.get_active_rules()))
{ }


bool PfStateSuccIterator::valid() const
{

}


ProofStatePtr PfStateSuccIterator::get() const
{

}


void PfStateSuccIterator::advance()
{

}


}  // namespace equational
}  // namespace logic
}  // namespace atp


