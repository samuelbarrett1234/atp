#pragma once


/**
\file

\author Samuel Barrett

\brief Contains interfaces for database objects which are containers
	of data, like indices and tables.
	
\details This does not include "simple" containers like DArray - the
	containers here are a bit more nontrivial and support iteration.

*/


#include <memory>
#include "../ATPDatabaseAPI.h"
#include "Data.h"
#include "DBIterators.h"


namespace atp
{
namespace db
{


class ILock;  // forward declaration


/**
\brief A classification of how well a container supports a particular
	kind of query.

\details A container either (i) doesn't support a particular type of
	query, (ii) supports it, however isn't the right structure to
	be able to do such an operation efficiently, and (iii) is a
	structure which has a fast implementation of a kind of query.
	The most interesting case is (ii) - for example: range lookups on
	a hash table. You can do them, but it won't be very good. This is
	to allow query optimisers to consider different ways of executing
	a query.
*/
enum class QuerySupport
{
	NONE, POOR, OPTIMISED
};


/**
\brief A structure for helping build queries.

\note Example usage: q.select().with_lock(lk).where_equal("col-name", 5)
*/
class ATP_DATABASE_API DBContainerQuery
{
public:
	// builder functions

	/**
	\brief Indicates that this is a selection operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	DBContainerQuery& select();

	/**
	\brief Indicates that this is an update operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	DBContainerQuery& update();
	
	/**
	\brief Indicates that this is a delete operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	DBContainerQuery& delete_rows();

	/**
	\brief Filter out such that we only iterate over results where
		col==value.

	\pre col.type() == value.type()

	\note This is taken to be `identical` for statement objects.

	\returns *this
	*/
	DBContainerQuery& where_equal(const Column& col, const DValue& value);

	/**
	\brief Filter out such that we only iterate over results where
		lower_val <= col <= upper_val.

	\pre lower_val.type() == upper_val.type() and neither have type
		STMT and col.type() == lower_val.type()

	\returns *this
	*/
	DBContainerQuery& where_between(const Column& col,
		const DValue& lower_val, const DValue& upper_val);

	/**
	\brief Filter out such that we only iterate over statements
		equivalent to `value`.

	\pre col.type() == value.type() and value.type() == STMT

	\returns *this
	*/
	DBContainerQuery& where_equivalent(const Column& col,
		const DValue& value);

	/**
	\brief Filter out such that we only iterate over statements
		which match `value` i.e. where there exists a free variable
		substitution producing `value`.

	\pre col.type() == value.type() and value.type() == STMT

	\returns *this
	*/
	DBContainerQuery& where_matching(const Column& col,
		const DValue& value);

	/**
	\brief Use the given lock to access the resources.

	\note If this is not specified, the container will attempt to
		obtain its own lock, and if this fails, the construction
		of the iterator will fail.
	*/
	DBContainerQuery& with_lock(
		std::shared_ptr<ILock> p_lock);

	// getter functions (after having been built)

	// TODO
};


/**
\brief A fundamental data container type. Think of this as a table-
	like object.
	
\details It has an ordered array of column names and data types. It
	allows the user to determine what kinds of queries are able to
	be performed on this container, and how efficient they might be.

\note All operations on this container are THREAD-SAFE.
*/
class ATP_DATABASE_API IDBContainer
{
public:
	virtual ~IDBContainer() = default;

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
	\returns The number of rows in this container.
	*/
	virtual size_t num_rows() const = 0;

	/**
	\brief Determine the support for the given query.

	\note If `q` contains a lock which does not cover the correct
		resources needed to carry out this query, NONE is returned.
	*/
	virtual QuerySupport get_support(
		const DBContainerQuery& q) const = 0;

	/**
	\brief Create an iterator to enumerate the results of the given
		query.

	\pre get_support(q) != QuerySupport::NONE

	\warning If you do not provide a lock with q, this function will
		try to obtain its own lock, which might easily fail.

	\returns nullptr on failure, otherwise a new iterator object.
	*/
	virtual std::shared_ptr<IDBIterator> begin_query(
		const DBContainerQuery& q) = 0;

	/**
	\brief Get the resources that would be needed to carry out the
		given query.

	\pre get_support(q) != QuerySupport::NONE

	\post The resource list returned is precisely the set of
		resources that has to be locked in order to carry this
		query out.
	*/
	virtual ResourceList get_resources(
		const DBContainerQuery& q) const = 0;
};


/**
\brief A container which has a special designated "key" column for
	which it is particularly good at performing queries on that value

\note All operations on this container are THREAD-SAFE.
*/
class ATP_DATABASE_API IDBKeyValueContainer :
	public virtual IDBContainer
{
public:
	virtual ~IDBKeyValueContainer() = default;

	/**
	\brief Get the key column.

	\note Generally, containers will support efficient operations on
		their key column.

	\post cols().contains(key_col())
	*/
	virtual Column key_col() const = 0;

	/**
	\brief Get the rest of the columns (the non-key ones) which are
		thought of as the "values".
	
	\post val_cols() is a subset of cols() and
		!val_cols().contains(key_col())
	*/
	virtual ColumnList val_cols() const = 0;
};


}  // namespace db
}  // namespace atp


