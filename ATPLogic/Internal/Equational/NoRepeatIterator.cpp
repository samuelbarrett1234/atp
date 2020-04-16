/**

\file

\author Samuel Barrett

*/


#include "NoRepeatIterator.h"
#include "Statement.h"
#include "ProofState.h"


namespace atp
{
namespace logic
{
namespace equational
{


PfStateSuccIterPtr NoRepeatIterator::construct(
	const ProofState& parent, PfStateSuccIterPtr iter)
{
	return std::make_shared<NoRepeatIterator>(parent,
		std::move(iter));
}


NoRepeatIterator::NoRepeatIterator(
	const ProofState& parent, PfStateSuccIterPtr iter) :
	m_parent(parent), m_iter(iter)
{
	forward();
}


bool NoRepeatIterator::valid() const
{
	return m_iter->valid();
}


ProofStatePtr NoRepeatIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	return m_iter->get();
}

void NoRepeatIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	m_iter->advance();
	forward();
}


size_t NoRepeatIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	return m_iter->size();
}


void NoRepeatIterator::forward()
{
	// keep advancing while we're valid and the forefront is blocked
	while (m_iter->valid() && blocked())
	{
		m_iter->advance();
	}
}


bool NoRepeatIterator::blocked() const
{
	ATP_LOGIC_PRECOND(m_iter->valid());

	ProofState* pstate = dynamic_cast<ProofState*>(
		m_iter->get().get());
	ATP_LOGIC_ASSERT(pstate != nullptr);
	auto stmt = pstate->forefront();

	auto p_list = m_parent.get_proof_list();
	ATP_LOGIC_ASSERT(p_list != nullptr);

	do
	{
		if (stmt.equivalent(p_list->head))
			return true;

		p_list = p_list->tail.get();
	} while (p_list != nullptr);

	return false;
}

}  // namespace equational
}  // namespace logic
}  // namespace atp


