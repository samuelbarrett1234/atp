/**
\file

\author Samuel Barrett

*/


#include "StmtSuccIterator.h"


namespace atp
{
namespace logic
{
namespace equational
{


StmtSuccIterator::StmtSuccIterator(const Statement& my_root,
	const ModelContext& ctx, const KnowledgeKernel& ker) :
	m_ctx(ctx), m_ker(ker), m_pf_state(m_ctx, m_ker, my_root, false,
		false), m_pf_st_iter(m_pf_state.succ_begin())
{
	if (m_pf_st_iter->valid())
	{
		m_current = std::make_unique<Statement>(get_current());
	}
}


void StmtSuccIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());

	m_pf_st_iter->advance();

	if (valid())
	{
		// update this
		m_current = std::make_unique<Statement>(get_current());
	}
}


bool StmtSuccIterator::valid() const
{
	return m_pf_st_iter->valid();
}


const IStatement& StmtSuccIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_current != nullptr);
	return *m_current;
}


StmtSuccIterPtr StmtSuccIterator::dive() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_current != nullptr);

	return std::make_shared<StmtSuccIterator>(*m_current,
		m_ctx, m_ker);
}


Statement StmtSuccIterator::get_current() const
{
	ATP_LOGIC_PRECOND(valid());

	auto _p_st = m_pf_st_iter->get();

	auto p_st = dynamic_cast<ProofState*>(_p_st.get());

	ATP_LOGIC_ASSERT(p_st != nullptr);

	return p_st->forefront();
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


