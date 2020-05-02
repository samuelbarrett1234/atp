#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an abstraction of a database transaction

*/


#include <string>
#include <memory>
#include <boost/optional.hpp>
#include <boost/variant2/variant.hpp>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


enum class TransactionState
{
	RUNNING, COMPLETED, FAILED
};


enum class DType
{
	BOOL, INT, STR, FLOAT,
	BINARY,  /* binary is just an arbitrary bunch of bytes */
	NONE  /* lack of a value */
};
typedef boost::variant2::variant<bool, int, std::string, float,
	std::vector<unsigned char>, boost::none_t> DValue;


/**
\interface ITransaction

\brief A transaction is any database operation which is either (i)
	not instantaneous, or (ii) can fail (at potentially any point).
*/
class ATP_DATABASE_API ITransaction
{
public:
	virtual ~ITransaction() = default;

	/**
	\brief Perform one more "unit of computation".

	\pre state() == RUNNING
	*/
	virtual void step() = 0;

	/**
	\brief Get the state of this transaction.
	*/
	virtual TransactionState state() const = 0;

	/**
	\brief Determine if the statement is waiting on resources, so
		cannot execute.

	\details This is usually as it is waiting for a lock. Whenever a
		transaction is waiting, its state is RUNNING. However, if you
		have called step() many times and it is still waiting(), you
		may wish to forget about this statement for a bit and try
		later.

	\note Just to reiterate: waiting() is not an error!

	\note The implementer is not obliged to ever return true for this
		function - it is just a hint to the code executing the
		transaction that it may want to put it to one side
		temporarily if it is waiting.

	\post if returns true(), state() returns RUNNING

	\returns True iff is waiting
	*/
	virtual bool waiting() const = 0;
};


/**
\brief A query transaction is a transaction which can return results.

\details Query transactions occasionally "hold values" during a
	particular step of execution. A query is never guaranteed to hold
	values. A query's values are changed when step() is called. For
	example, the query's values may represent the current row.
*/
class ATP_DATABASE_API IQueryTransaction :
	public ITransaction
{
public:
	/**
	\brief Determine if the query has a row value at the current time
		(this function is constant between step() calls).

	\pre state() == RUNNING.
	*/
	virtual bool has_values() const = 0;

	/**
	\brief Get the arity (i.e. number of columns) of the current set
		of values.

	\pre has_values()
	*/
	virtual size_t arity() const = 0;

	/**
	\brief Try to get the value at the given index.

	\param idx The column index.

	\param dtype The type to aim for.

	\param p_out_val Optional: if this is not null, and if the
		conversion was successful, the value will be written out
		here.

	\pre has_values() && idx < arity()

	\returns True iff the conversion was successful.
	*/
	virtual bool try_get(size_t idx, DType dtype,
		DValue* p_out_val = nullptr) const = 0;
};


}  // namespace db
}  // namespace atp


