#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of IQueryTransaction for SQLite
	databases.

*/


#include <boost/noncopyable.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/ITransaction.h"


struct sqlite3;  // forward declaration
struct sqlite3_stmt;  // forward declaration


namespace atp
{
namespace db
{


/**
\brief All SQLite queries can (at least try to) get values.
*/
class ATP_DATABASE_API SQLiteQueryTransaction :
	public IQueryTransaction
{
public:
	/**
	\param db A weakly-held pointer to the database connection (where
		"weakly-held" means that this object is NOT responsible for
		deleting it)

	\param stmt A strongly-held pointer to the statement object
		(where "strongly-held" means that we assume ownership of this
		object hereafter).

	\pre db != nullptr && stmt != nullptr
	*/
	SQLiteQueryTransaction(sqlite3* db, sqlite3_stmt* stmt);
	~SQLiteQueryTransaction();

	void step() override;
	inline TransactionState state() const override
	{
		return m_state;
	}
	inline bool waiting() const override
	{
		return m_waiting;
	}
	bool has_values() const override;
	size_t arity() const override;
	bool try_get(size_t idx, DType dtype,
		DValue* p_out_val = nullptr) const override;

private:
	sqlite3* m_db;
	sqlite3_stmt* m_stmt;
	TransactionState m_state;
	bool m_waiting;
};


}  // namespace db
}  // namespace atp


