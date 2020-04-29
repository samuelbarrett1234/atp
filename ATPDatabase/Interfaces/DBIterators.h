#pragma once


/**
\file

\author Samuel Barrett

\brief Contains interfaces for objects which iterate over rows of the
	database.

*/


#include "../ATPDatabaseAPI.h"
#include "Data.h"


namespace atp
{
namespace db
{


/**
\interface IDBIterator

\brief An interface for iterating over rows of a database.

\warning Since database operations are intended to occur on a single
	thread, there is no reason to make these iterators thread-safe,
	hence they are NOT in general. Iterators should not be shared.

\note Iterators will somehow need to gain locks to the data they are
	iterating over. This will be provided by the parent container,
	but just remember that these iterators almost certainly have a
	lock inside them, so only keep them alive for as long as you need
	to.
*/
class ATP_DATABASE_API IDBIterator
{
public:
	virtual ~IDBIterator() = default;

	/**
	\brief Advance this iterator onto the next element

	\pre valid()
	*/
	virtual void advance() = 0;

	/**
	\brief Determine if this is an end iterator or not.

	\returns True if this iterator is valid, false if it is an end
		iterator.
	*/
	virtual bool valid() const = 0;
};


/**
\interface IDBSelectIterator

\brief This iterator has the ability to "get" values it iterates over
*/
class ATP_DATABASE_API IDBSelectIterator :
	public virtual IDBIterator
{
public:
	virtual ~IDBSelectIterator() = default;

	/**
	\brief Get the columns that appear in the results of this select
		iterator.
	*/
	virtual ColumnList cols() const = 0;

	/**
	\brief Equivalent to cols().size()
	*/
	virtual size_t num_cols() const = 0;

	/**
	\brief Get the data type of the given column

	\pre cols().contains(col)
	*/
	virtual DType type(const Column& col) const = 0;

	/**
	\brief Get the value in the given column at the row which this
		iterator is currently pointing to.

	\pre cols().contains(col)

	\pre valid()
	*/
	virtual DValue get(const Column& col) const = 0;

	/**
	\brief Return the entire row (columns are given in the same order
		as returned in cols()).

	\pre valid()
	*/
	virtual std::vector<DValue> get_all() const = 0;
};


/**
\brief Extends the select iterator with the ability to change any of
	all the values pointed to under the given row.
*/
class ATP_DATABASE_API IDBUpdateIterator :
	public virtual IDBSelectIterator
{
public:
	virtual ~IDBUpdateIterator() = default;

	/**
	\brief Determine if the given column is mutable (so you are
		allowed to change the values under it).

	\note This function will remain constant throughout the execution
		of the iterator (i.e. it does not depend on position).

	\returns True if the column can be modified, false if not.

	\pre cols().contains(col)
	*/
	virtual bool is_mutable(const Column& col) const = 0;

	/**
	\brief Determine if all columns are mutable

	\returns forall col in cols() : is_mutable(col)
	*/
	virtual bool all_mutable() const = 0;

	/**
	\brief Set the value in the given column at the row which this
		iterator is currently pointing to.

	\pre cols().contains(col) && is_mutable(col)
		&& type(col) == value.type()

	\pre valid()

	\warning There is no way to undo changes!
	*/
	virtual void set(const Column& col, const DValue& value) = 0;

	/**
	\brief Update the entire row.

	\pre all_mutable()

	\pre The types of `values` agree with the types in the (ordered
		list of) columns. This necessarily means that values.size()
		== cols().size().

	\pre valid()

	\warning There is no way to undo changes!
	*/
	virtual void set_all(const std::vector<DValue>& values) = 0;
};


/**
\brief Extends the base iterator with the ability to insert the
	data it holds.

\warning This iterator's `advance` function will **SKIP** inserting
	the current row. There is very little reason to do this, however
	that implementation of `advance` would be most true to its
	declaration.
*/
class ATP_DATABASE_API IDBInsertIterator :
	public virtual IDBIterator
{
public:
	virtual ~IDBInsertIterator() = default;

	/**
	\brief Insert the current row of data, and then move onto the
		next one.

	\details If this was the last row to insert, then the insert
		iterator will be made invalid after calling this.

	\pre valid()
	*/
	virtual void insert_advance() = 0;
};


/**
\brief Extends the select iterator with the ability to delete the
	current row.

\warning There is no going back after a row is deleted.

\warning This iterator's `advance` function will **SKIP** deleting
	the current row. There is very little reason to do this, however
	that implementation of `advance` would be most true to its
	declaration.
*/
class ATP_DATABASE_API IDBDeleteIterator :
	public virtual IDBSelectIterator
{
public:
	virtual ~IDBDeleteIterator() = default;

	/**
	\brief Delete the current row, and then advance to the next one.

	\details If this was the last row to delete, then the delete
		iterator will be made invalid after calling this.

	\pre valid()

	\warning There is no way to undo this!
	*/
	virtual void delete_advance() = 0;
};


}  // namespace db
}  // namespace atp


