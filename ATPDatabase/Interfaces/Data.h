#pragma once


/**
\file

\author Samuel Barrett

\brief Contains helper objects to make it easier for the user to
	handle a lack of type safety
*/


#include <string>
#include <vector>
#include <boost/any.hpp>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


// for locking etc
typedef size_t ResourceName;
typedef std::vector<ResourceName> ResourceList;


/**
\brief An enumeration of the data types supported by this library
*/
enum class DType
{
	// basic
	INT, UINT, FLOAT, STR,

	// the concrete implementation of a statement of course depends
	// on the underlying logic!
	STMT
};


/**
\brief Represents an arbitrary data element in this library.

\details It can represent any value in the `DType` enumeration. Note
	that, if this data value represents a statement, the underlying
	object in DValue is a std::shared_ptr<logic::Statement>!!!

\see DType
*/
struct ATP_DATABASE_API DValue
{
	DType type() const;

	int as_int() const;
	size_t as_uint() const;
	float as_float() const;
	const std::string& as_str() const;
	const std::shared_ptr<logic::IStatement>& as_stmt() const;

	inline operator int() const
	{
		return as_int();
	}
	inline operator size_t() const
	{
		return as_uint();
	}
	inline operator float() const
	{
		return as_float();
	}
	inline operator const std::string&() const
	{
		return as_str();
	}
	inline const logic::IStatement* operator->() const
	{
		return as_stmt().get();
	}
};


/**
\brief Represents an array of DValues, where all of the DValues have
	the same type (it is homogeneous, like normal arrays).

\details This is of course, more efficient than an array of DValues
	when you know all of the types are the same. It can be used to
	efficiently represent a column, say.
*/
struct ATP_DATABASE_API DArray
{
	DType type() const;
	size_t size() const;
	bool empty() const;
	DValue val_at(size_t idx) const;
};


/**
\brief Represents a column name (or index)

\details This column object doesn't necessarily have to be tied to a
	specific table, however it is more efficient when tied to a table
	because it stores the index of the column rather than the name.
*/
struct ATP_DATABASE_API Column
{
	std::string name() const;
};


/**
\brief Represents a list of column objects.

\invariant Either none of these columns are tied to a table, or all
	of them are tied to the same table.

\details If none of them are tied to a table, this is effectively
	just a vector of strings. If they are all tied to the same table
	this is instead basically just a vector of indices of the columns
	that are selected.
*/
struct ATP_DATABASE_API ColumnList
{
	bool contains(const Column& col) const;
	size_t size() const;
	bool empty() const;
};


}  // namespace db
}  // namespace atp


