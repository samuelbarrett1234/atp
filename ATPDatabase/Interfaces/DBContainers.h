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
#include <boost/optional.hpp>
#include <boost/bind.hpp>
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
	enum class QueryKind
	{
		SELECT, UPDATE, DELETE, INSERT
	};
	enum class WhereKind
	{
		FIND_ALL, FIND_VALUE, FIND_RANGE,
		FIND_MATCHING, FIND_EQUIVALENT
	};
	enum class TraversalMode
	{
		// how to enumerate the results of the query?
		DEFAULT, RANDOM, ASCENDING, DESCENDING
	};

	// builder functions

	/**
	\brief Indicates that this is a selection operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	inline DBContainerQuery& select()
	{
		m_kind = QueryKind::SELECT;
		return *this;
	}

	/**
	\brief Indicates that this is an update operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	inline DBContainerQuery& update()
	{
		m_kind = QueryKind::UPDATE;
		return *this;
	}

	/**
	\brief Insert a single given row of data.

	\details Ignores any ordering / "where". The datatypes of the
		row data need to match up with the table's columns if the
		insert operation is to be supported.

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	inline DBContainerQuery& insert(std::vector<DValue> row)
	{
		ATP_DATABASE_PRECOND(!row.empty());
		m_kind = QueryKind::INSERT;
		m_insert_row = std::move(row);
		m_insert_rows.reset();
		return *this;
	}

	/**
	\brief Insert many rows of data.

	\details Ignores any ordering / "where". The datatypes of the
		rows data need to match up with the table's columns if the
		insert operation is to be supported.

	\param rows The rows to insert, in column-major order.

	\pre Every DArray in `rows` must have the same size (and that
		size is equal to the number of elements being inserted).

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	inline DBContainerQuery& insert(std::vector<DArray> rows)
	{
		ATP_DATABASE_PRECOND(!rows.empty());

		// check all the arrays have the same size
		const size_t num_rows = rows.front().size();
		ATP_DATABASE_PRECOND(std::all_of(rows.begin(), rows.end(),
			boost::bind(&DArray::size, _1) == num_rows));

		m_kind = QueryKind::INSERT;
		m_insert_row.reset();
		m_insert_rows = std::move(rows);
		return *this;
	}
	
	/**
	\brief Indicates that this is a delete operation

	\note Overrides any previous calls to select/update/delete

	\returns *this
	*/
	inline DBContainerQuery& delete_rows()
	{
		m_kind = QueryKind::DELETE;
		return *this;
	}

	/**
	\brief Apply the select/update/delete to all rows.
	*/
	inline DBContainerQuery& all()
	{
		m_val1.reset();
		m_val2.reset();
		m_col.reset();
		m_where = WhereKind::FIND_ALL;
		return *this;
	}

	/**
	\brief Filter out such that we only iterate over results where
		col==value.

	\pre col.type() == value.type()

	\note This is taken to be `identical` for statement objects.

	\returns *this
	*/
	inline DBContainerQuery& where_equal(const Column& col,
		const DValue& value)
	{
		m_col = col;
		m_val1 = value;
		m_val2.reset();
		m_where = WhereKind::FIND_VALUE;
		return *this;
	}

	/**
	\brief Filter out such that we only iterate over results where
		lower_val <= col <= upper_val.

	\pre lower_val.type() == upper_val.type() and neither have type
		STMT and col.type() == lower_val.type()

	\returns *this
	*/
	inline DBContainerQuery& where_between(const Column& col,
		const DValue& lower_val, const DValue& upper_val)
	{
		m_where = WhereKind::FIND_RANGE;
		m_col = col;
		m_val1 = lower_val;
		m_val2 = upper_val;
		return *this;
	}

	/**
	\brief Filter out such that we only iterate over statements
		equivalent to `value`.

	\pre col.type() == value.type() and value.type() == STMT

	\returns *this
	*/
	inline DBContainerQuery& where_equivalent(const Column& col,
		const DValue& value)
	{
		m_col = col;
		m_val1 = value;
		m_val2.reset();
		m_where = WhereKind::FIND_EQUIVALENT;
		return *this;
	}

	/**
	\brief Filter out such that we only iterate over statements
		which match `value` i.e. where there exists a free variable
		substitution producing `value`.

	\pre col.type() == value.type() and value.type() == STMT

	\returns *this
	*/
	inline DBContainerQuery& where_matching(const Column& col,
		const DValue& value)
	{
		m_col = col;
		m_val1 = value;
		m_val2.reset();
		m_where = WhereKind::FIND_MATCHING;
		return *this;
	}

	/**
	\brief Enumerate the results of the query in their "default"
		order, which is just the order they are represented in the
		table.

	\returns *this
	*/
	inline DBContainerQuery& default_order()
	{
		m_trav = TraversalMode::DEFAULT;
		m_sort_col.reset();
		return *this;
	}

	/**
	\brief Enumerate the results of the query in a shuffled, random
		order.

	\warning The implementation is free to approximate this operation
		and it doesn't have to be a truly random shuffle, as that may
		be hard to implement efficiently.

	\returns *this
	*/
	inline DBContainerQuery& shuffle()
	{
		m_trav = TraversalMode::RANDOM;
		m_sort_col.reset();
		return *this;
	}

	/**
	\brief Enumerate the results of this query in descending order
		of this column.

	\note The column will (probably) have to have numeric type, or
		at least not Statement type, to be orderable. However this
		just depends on the container, so check support first. But
		if you are unexpectedly getting no support for your query,
		it may be because you're trying to sort by a statement-valued
		column.

	returns *this.
	*/
	inline DBContainerQuery& sort_descending_by(const Column& col)
	{
		m_trav = TraversalMode::DESCENDING;
		m_sort_col = col;
		return *this;
	}

	/**
	\brief Enumerate the results of this query in ascending order
		of this column.

	\note The column will (probably) have to have numeric type, or
		at least not Statement type, to be orderable. However this
		just depends on the container, so check support first. But
		if you are unexpectedly getting no support for your query,
		it may be because you're trying to sort by a statement-valued
		column.

	returns *this.
	*/
	inline DBContainerQuery& sort_ascending_by(const Column& col)
	{
		m_trav = TraversalMode::ASCENDING;
		m_sort_col = col;
		return *this;
	}

	/**
	\brief Use the given lock to access the resources.

	\note If this is not specified, the container will attempt to
		obtain its own lock, and if this fails, the construction
		of the iterator will fail.
	*/
	inline DBContainerQuery& with_lock(
		std::shared_ptr<ILock> p_lock)
	{
		m_lock = p_lock;
		return *this;
	}

	// getter functions (after having been built)

	inline QueryKind query_kind() const
	{
		return m_kind;
	}
	inline WhereKind where_kind() const
	{
		return m_where;
	}
	inline TraversalMode traversal_mode() const
	{
		return m_trav;
	}
	inline const std::shared_ptr<ILock>& lock() const
	{
		return m_lock;
	}
	inline DValue val1() const
	{
		ATP_DATABASE_PRECOND(m_val1.has_value());
		return *m_val1;
	}
	inline DValue val2() const
	{
		ATP_DATABASE_PRECOND(m_val2.has_value());
		return *m_val2;
	}
	inline Column col() const
	{
		ATP_DATABASE_PRECOND(m_col.has_value());
		return *m_col;
	}
	inline Column sort_by_col() const
	{
		ATP_DATABASE_PRECOND(m_trav == TraversalMode::ASCENDING
			|| m_trav == TraversalMode::DESCENDING);
		return *m_sort_col;
	}
	inline bool is_inserting_many_rows() const
	{
		ATP_DATABASE_PRECOND(m_kind == QueryKind::INSERT);

		// exactly one of these should be nonempty:
		ATP_DATABASE_ASSERT(m_insert_row.has_value() !=
			m_insert_rows.has_value());

		return m_insert_rows.has_value();
	}
	inline const std::vector<DValue>& row() const
	{
		ATP_DATABASE_PRECOND(m_kind == QueryKind::INSERT);
		ATP_DATABASE_PRECOND(!is_inserting_many_rows());
		return *m_insert_row;
	}
	inline const std::vector<DArray>& rows() const
	{
		ATP_DATABASE_PRECOND(m_kind == QueryKind::INSERT);
		ATP_DATABASE_PRECOND(is_inserting_many_rows());
		return *m_insert_rows;
	}

private:
	QueryKind m_kind = QueryKind::SELECT;
	WhereKind m_where = WhereKind::FIND_ALL;
	TraversalMode m_trav = TraversalMode::DEFAULT;
	std::shared_ptr<ILock> m_lock;

	// for comparisons, if applicable
	boost::optional<DValue> m_val1, m_val2;
	boost::optional<Column> m_col, m_sort_col;
	boost::optional<std::vector<DValue>> m_insert_row;
	boost::optional<std::vector<DArray>> m_insert_rows;
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
\brief See the column decorator container interface.
*/
enum class ColumnFlag
{
	// if this is set, it *does not allow* insertions that cause
	// duplicate values of this column
	UNIQUE,

	// if this is set, it automatically implies UNIQUE, and its
	// behaviour is as follows: row insertions ignore this column,
	// and we automatically generate values for it.
	// warning: we override the value provided in `insert`.
	// note: at most one column can have the AUTO_KEY flag.
	AUTO_KEY

	// examples of what could be added:
	// SORTED,  // hint to store data in sorted order
	// ALLOW_NULLS
};


/**
\brief A container which has extra information about each column, and
	supports various column "decorators" (just additional info about
	a column)

\note All operations on this container are THREAD-SAFE.
*/
class ATP_DATABASE_API IDBColumnDecoratorContainer :
	public virtual IDBContainer
{
public:
	virtual ~IDBColumnDecoratorContainer() = default;

	/**
	\brief Does any column in this container have the AUTO_KEY flag?

	\note Generally, containers will support efficient operations on
		their auto key column.

	\post any(cols(), has_col_flag(AUTO_KEY, col)) == true
	*/
	virtual bool has_autokey_col() const = 0;

	/**
	\brief If we have an autokey column, it is unique, so this
		function will return it.

	\pre has_autokey_col()

	\post has_col_flag(AUTO_KEY, get_autokey_col()) == true
	*/
	virtual Column get_autokey_col() const = 0;

	/**
	\brief Does the given column have the given flag?

	\pre cols().contains(col)
	*/
	virtual bool has_col_flag(ColumnFlag cf,
		const Column& col) const = 0;
};


}  // namespace db
}  // namespace atp


