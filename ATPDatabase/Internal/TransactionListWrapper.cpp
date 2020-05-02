/*
\file

\author Samuel Barrett

*/


#include "TransactionListWrapper.h"


namespace atp
{
namespace db
{


TransactionListWrapper::TransactionListWrapper(
	std::vector<std::unique_ptr<ITransaction>> list) :
	m_transaction_list(std::move(list)),
	m_idx(0)
{
	// none of them should be null
	ATP_DATABASE_PRECOND(std::none_of(m_transaction_list.begin(),
		m_transaction_list.end(), [](const auto& p)
		{ return p == nullptr; }));

	// might have to go forwards if any of them begin in a completed
	// state.
	forward();
}


void TransactionListWrapper::step()
{
	ATP_DATABASE_PRECOND(state() == TransactionState::RUNNING);
	ATP_DATABASE_ASSERT(m_transaction_list[m_idx] != nullptr);

	m_transaction_list[m_idx]->step();

	forward();
}


TransactionState TransactionListWrapper::state() const
{
	if (m_idx == m_transaction_list.size())
		return TransactionState::COMPLETED;

	ATP_DATABASE_ASSERT(m_transaction_list[m_idx] != nullptr);

	const auto result_state = m_transaction_list[m_idx]->state();

	// invariant
	ATP_DATABASE_ASSERT(result_state != TransactionState::COMPLETED);

	return result_state;
}


bool TransactionListWrapper::waiting() const
{
	ATP_DATABASE_PRECOND(state() == TransactionState::RUNNING);

	ATP_DATABASE_ASSERT(m_idx < m_transaction_list.size());
	ATP_DATABASE_ASSERT(m_transaction_list[m_idx] != nullptr);

	return m_transaction_list[m_idx]->waiting();
}


void TransactionListWrapper::forward()
{
	// forward until next valid transaction, or end
	while (m_idx < m_transaction_list.size() &&
		m_transaction_list[m_idx]->state() ==
		TransactionState::COMPLETED)
	{
		ATP_DATABASE_ASSERT(m_transaction_list[m_idx] != nullptr);

		m_transaction_list[m_idx].reset();
		++m_idx;
	}
}


}  // namespace db
}  // namespace atp


