/**
\file

\author Samuel Barrett

*/


#include <sqlite3.h>
#include "SQLiteTransaction.h"


namespace atp
{
namespace db
{


SQLiteQueryTransaction::SQLiteQueryTransaction(sqlite3* db,
	sqlite3_stmt* stmt) :
	m_db(db), m_stmt(stmt), m_state(TransactionState::RUNNING),
	m_waiting(false)
{
	ATP_DATABASE_PRECOND(db != nullptr);
	ATP_DATABASE_PRECOND(stmt != nullptr);
}


SQLiteQueryTransaction::~SQLiteQueryTransaction()
{
	sqlite3_finalize(m_stmt);
}


void SQLiteQueryTransaction::step()
{
	ATP_DATABASE_PRECOND(state() == TransactionState::RUNNING);

	const int rc = sqlite3_step(m_stmt);

	switch (rc)
	{
	case SQLITE_DONE:
		m_state = TransactionState::COMPLETED;
		break;
	case SQLITE_ROW:  // running
		m_waiting = false;
		break;
	case SQLITE_BUSY:  // running
		m_waiting = true;
		break;
	default:
		m_state = TransactionState::FAILED;
		break;
	}
}


bool SQLiteQueryTransaction::has_values() const
{
	ATP_DATABASE_PRECOND(state() == TransactionState::RUNNING);

	// this function returns true iff m_stmt has been stepped at
	// least once
	return sqlite3_stmt_busy(m_stmt) != 0;
}


size_t SQLiteQueryTransaction::arity() const
{
	ATP_DATABASE_PRECOND(has_values());
	return sqlite3_column_count(m_stmt);
}


bool SQLiteQueryTransaction::try_get(size_t idx, DType dtype,
	DValue* p_out_val) const
{
	ATP_DATABASE_PRECOND(has_values());
	ATP_DATABASE_PRECOND(idx < arity());

	switch (dtype)
	{
	case DType::BOOL:
	{
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_INTEGER)
			return false;

		const int result = sqlite3_column_int(m_stmt, (int)idx);
		switch (result)
		{
		case 0:
			if (p_out_val) *p_out_val = false;
			return true;
		case 1:
			if (p_out_val) *p_out_val = true;
			return true;
		default:
			return false;
		}
	}

	case DType::INT:
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_INTEGER)
			return false;
		if (p_out_val)
			*p_out_val = sqlite3_column_int(m_stmt, (int)idx);
		return true;

	case DType::STR:
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_TEXT)
			return false;
		if (p_out_val)
			*p_out_val = std::string((const char*)
				sqlite3_column_text(m_stmt, (int)idx));
		return true;

	case DType::FLOAT:
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_FLOAT)
			return false;
		if (p_out_val)
			*p_out_val = (float)sqlite3_column_double(m_stmt,
				(int)idx);
		return true;

	case DType::BINARY:
	{
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_BLOB)
			return false;

		if (p_out_val)
		{
			const void* p_mem = sqlite3_column_blob(m_stmt, (int)idx);
			const size_t size = sqlite3_column_bytes(m_stmt, (int)idx);

			std::vector<unsigned char> buffer;
			buffer.resize(size);

			memcpy_s(buffer.data(), buffer.size(), p_mem, size);

			*p_out_val = std::move(buffer);
		}

		return true;
	}

	case DType::NONE:
	{
		if (sqlite3_column_type(m_stmt, (int)idx) != SQLITE_NULL)
			return false;

		if (p_out_val)
			*p_out_val = boost::none;

		return true;
	}

	default:
		ATP_DATABASE_PRECOND(false && "invalid DType!");
		return false;
	}
}


}  // namespace db
}  // namespace atp


