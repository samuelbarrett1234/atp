#pragma once

/*
\file

\author Samuel Barrett

*/


#include <vector>
#include <memory>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/ITransaction.h"


namespace atp
{
namespace db
{


/**
\brief This object is a wrapper for a list of transactions.

\details If any transaction fails, the list fails. Once the "current"
	transaction enters the completed state, we move on to the next
	one.
*/
class TransactionListWrapper :
	public ITransaction
{
public:
	/**
	\brief This class allows you to build transaction list objects.
	*/
	struct Builder
	{
		inline void add(std::unique_ptr<ITransaction> p)
		{
			m_transaction_list.emplace_back(std::move(p));

			ATP_DATABASE_PRECOND(m_transaction_list.back()
				!= nullptr);
		}

		inline std::unique_ptr<ITransaction> build()
		{
			return std::make_unique<TransactionListWrapper>(
				std::move(m_transaction_list));
		}

		std::vector<std::unique_ptr<ITransaction>> m_transaction_list;
	};

public:
	// please construct using the builder instead
	TransactionListWrapper(std::vector<std::unique_ptr<ITransaction>> list);

public:  // interface functions

	void step() override;
	TransactionState state() const override;
	bool waiting() const override;

private:
	void forward();

private:
	// invariant: if m_idx < size then m_transaction_list[m_idx] is
	// not completed
	size_t m_idx;
	std::vector<std::unique_ptr<ITransaction>> m_transaction_list;
};


}  // namespace db
}  // namespace atp


