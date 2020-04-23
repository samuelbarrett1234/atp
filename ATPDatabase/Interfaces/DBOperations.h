#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the interface of a database operation.

*/


#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


/**
\brief Represents an operation (e.g. a query) which runs on a
	database over potentially many timesteps.

\details DB operations will almost certainly involve access to some
	resources, hence need locks. If you have a DBOperation object,
	that implies it has already got the locks it needs. You may need
	to try several times to successfully create such an object though
	to ensure it gets the access it needs.
*/
class ATP_DATABASE_API IDBOperation
{
public:
	virtual ~IDBOperation() = default;

	/**
	\brief Do another step of the computation.

	\pre !done()
	*/
	virtual void tick() = 0;

	/**
	\brief Determine if the operation has finished, and the results
		are ready.

	\returns True iff done.
	*/
	virtual bool done() const = 0;
};
typedef std::unique_ptr<IDBOperation> DBOpPtr;


/**
\brief Many database operations perform reading, and collect results.
	This kind of operation provides an interface for getting such
	results.

\warning Almost all of this extended interface has the precondition
	`done()` as the results will only be ready when the underlying
	operation finishes.
*/
class ATP_DATABASE_API IDBReadOperation :
	public virtual IDBOperation
{
public:
	virtual ~IDBReadOperation() = default;

	/**
	\brief The number of rows read by the query.

	\pre done()
	*/
	virtual size_t num_rows_read() const = 0;

	/**
	\brief Equivalent to cols().size()

	\pre done()
	*/
	virtual size_t num_cols() const = 0;

	/**
	\brief Get the columns that result from the operation.

	\note This can be called before the operation finishes, if you
		wish.
	*/
	virtual ColumnList cols() const = 0;

	/**
	\brief Extract the entire ith column

	\pre i < num_cols() && done()

	\returns All the values for the column.

	\post The order of the values within the returned array are the
		same as if one were to call .at(j, i) for j = 0,1,...
	*/
	virtual DArray col_values(size_t i) const = 0;

	/**
	\brief Returns a particular value from the table.

	\pre row_idx < num_rows_read() && col_idx < num_cols() && done()
	*/
	virtual DValue at(size_t row_idx, size_t col_idx) const = 0;

	/**
	\brief Extract an entire row of data.

	\pre i < num_rows_read() && done()

	\post Results returned are in column-order given by `cols()`
	*/
	virtual std::vector<DValue> row_values(size_t i) const = 0;
};


}  // namespace db
}  // namespace atp



