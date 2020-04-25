#pragma once


/**
\file

\author Samuel Barrett

\brief Contains helper objects to make it easier for the user to
	handle a lack of type safety
*/


#include <string>
#include <vector>
#include <algorithm>
#include <boost/variant2/variant.hpp>
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
class ATP_DATABASE_API DValue
{
public:
	DValue(int x) :
		m_type(DType::INT),
		m_data(x)
	{ }
	DValue(size_t x) :
		m_type(DType::UINT),
		m_data(x)
	{ }
	DValue(float x) :
		m_type(DType::FLOAT),
		m_data(x)
	{ }
	DValue(std::string x) :
		m_type(DType::STR),
		m_data(x)
	{ }
	DValue(std::shared_ptr<logic::IStatement> x) :
		m_type(DType::STMT),
		m_data(std::move(x))
	{ }
	DValue(const logic::IStatement& x);

	inline DType type() const
	{
		return m_type;
	}

	inline int as_int() const
	{
		ATP_DATABASE_PRECOND(m_type == DType::INT);
		return boost::variant2::get<0>(m_data);
	}
	inline size_t as_uint() const
	{
		ATP_DATABASE_PRECOND(m_type == DType::UINT);
		return boost::variant2::get<1>(m_data);
	}
	inline float as_float() const
	{
		ATP_DATABASE_PRECOND(m_type == DType::FLOAT);
		return boost::variant2::get<2>(m_data);
	}
	inline const std::string& as_str() const
	{
		ATP_DATABASE_PRECOND(m_type == DType::STR);
		return boost::variant2::get<3>(m_data);
	}
	inline const std::shared_ptr<logic::IStatement>& as_stmt() const
	{
		ATP_DATABASE_PRECOND(m_type == DType::STMT);
		return boost::variant2::get<4>(m_data);
	}

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

private:
	DType m_type;
	boost::variant2::variant<int, size_t, float, std::string,
		std::shared_ptr<logic::IStatement>> m_data;
};


/**
\brief Represents an array of DValues, where all of the DValues have
	the same type (it is homogeneous, like normal arrays).

\details This is of course, more efficient than an array of DValues
	when you know all of the types are the same. It can be used to
	efficiently represent a column, say.
*/
class ATP_DATABASE_API DArray
{
public:
	/**
	\brief Boost variant doesn't seem to like having a reference
		type in its union, so we create a separate struct to
		help this.
	*/
	struct StmtArrRef
	{
		StmtArrRef(logic::IStatementArray& ref) :
			ref(ref)
		{ }

		inline size_t size() const
		{
			return ref.size();
		}
		inline bool empty() const
		{
			return ref.empty();
		}

		logic::IStatementArray& ref;
	};

public:
	DArray(std::vector<int> arr) :
		m_type(DType::INT),
		m_data(std::move(arr))
	{ }
	DArray(std::vector<size_t> arr) :
		m_type(DType::UINT),
		m_data(std::move(arr))
	{ }
	DArray(std::vector<float> arr) :
		m_type(DType::FLOAT),
		m_data(std::move(arr))
	{ }
	DArray(std::vector<std::string> arr) :
		m_type(DType::STR),
		m_data(std::move(arr))
	{ }
	DArray(logic::StatementArrayPtr arr) :
		m_type(DType::INT),
		m_maybe_arr(std::move(arr)),
		m_data(*m_maybe_arr)
	{ }
	DArray(const std::vector<DValue>& arr);

	/**
	\brief Helper function for converting DArray into the logic's
		statement array type.

	\pre d_arr.type() == DType::STMT
	*/
	static inline logic::StatementArrayPtr to_stmt_arr(const DArray& d_arr)
	{
		ATP_DATABASE_PRECOND(d_arr.m_type == DType::STMT);
		ATP_DATABASE_ASSERT(d_arr.m_maybe_arr != nullptr);
		return d_arr.m_maybe_arr;
	}

	inline DType type() const
	{
		return m_type;
	}
	inline size_t size() const
	{
		return boost::variant2::visit([](const auto& arr)
			{ return arr.size(); }, m_data);
	}
	inline bool empty() const
	{
		return boost::variant2::visit([](const auto& arr)
			{ return arr.empty(); }, m_data);
	}

	// this is efficient for most data types, however it is actually
	// kind of slow for statements :(
	// (convert it to a statementarray in that case, preferably)
	DValue val_at(size_t idx) const;

private:
	// note that, if we are storing arrays of statements, we use the
	// logic's optimised statement array version. However, to make
	// the array interface in `m_data` uniform, we store a REFERENCE
	// in `m_data` rather than a pointer. In order to manage memeory
	// properly we need to store the pointer elsewhere.
	// In other words, if this is not null, then
	// m_data == *m_maybe_arr
	logic::StatementArrayPtr m_maybe_arr;

	DType m_type;
	boost::variant2::variant<
		std::vector<int>,
		std::vector<size_t>,
		std::vector<float>,
		std::vector<std::string>,
		StmtArrRef> m_data;
};


/**
\brief Represents a column name (or index)

\details This column object doesn't necessarily have to be tied to a
	specific table, however it is more efficient when tied to a table
	because it stores the index of the column rather than the name.
*/
class ATP_DATABASE_API Column
{
private:
	friend class ColumnList;
	static const constexpr size_t no_index = static_cast<size_t>(-1);

public:
	Column(size_t idx) :
		m_idx(idx),
		m_name_array(nullptr)
	{ }
	Column(const char* name) :
		m_idx(no_index),
		m_name(name),
		m_name_array(nullptr)
	{ }
	Column(std::string name) :
		m_idx(no_index),
		m_name(std::move(name)),
		m_name_array(nullptr)
	{ }
	Column(size_t idx,
		const std::vector<std::string>* name_array) :
		m_idx(idx), m_name_array(name_array)
	{ }

	inline std::string name() const
	{
		ATP_DATABASE_PRECOND(has_name());

		if (m_name_array)
		{
			ATP_DATABASE_ASSERT(m_idx < m_name_array->size());
			return m_name_array->at(m_idx);
		}
		else
		{
			ATP_DATABASE_ASSERT(m_idx == no_index);
			return m_name;
		}
	}
	inline size_t index() const
	{
		ATP_DATABASE_PRECOND(has_index());
		ATP_DATABASE_ASSERT(m_name_array == nullptr ||
			m_idx < m_name_array->size());

		return m_idx;
	}

	inline bool has_name() const
	{
		return ((m_name_array != nullptr) == (m_idx != no_index));
	}
	inline bool has_index() const
	{
		return (m_idx != no_index);
	}

	inline operator std::string() const
	{
		return name();
	}
	inline operator size_t() const
	{
		return index();
	}

private:
	// invariant: only one of m_idx and m_name is used, and if
	// m_name_array != nullptr then m_idx is used and m_name isn't
	// (m_idx == no_index when it isn't used)

	size_t m_idx;
	std::string m_name;
	const std::vector<std::string>* m_name_array;
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
class ATP_DATABASE_API ColumnList
{
public:
	ColumnList() :
		m_other_name_array(nullptr)
	{ }
	ColumnList(std::vector<std::string> names) :
		m_name_array(std::move(names))
	{ }
	ColumnList(std::vector<size_t> indices,
		const std::vector<std::string>* other_name_array) :
		m_indices(std::move(indices)),
		m_other_name_array(other_name_array)
	{
		ATP_DATABASE_PRECOND(m_other_name_array != nullptr);
	}

	bool contains(const Column& col) const;
	inline size_t size() const
	{
		ATP_DATABASE_ASSERT(m_name_array.empty() !=
			m_indices.empty());
		// exactly one of these should be empty anyway
		// (see above assertion)
		return std::max(m_indices.size(),
			m_name_array.size());
	}
	inline bool empty() const
	{
		ATP_DATABASE_ASSERT(m_name_array.empty() !=
			m_indices.empty());
		// exactly one of these should be empty anyway
		// (see above assertion)
		return m_name_array.empty() && m_indices.empty();
	}
	void insert(const Column& col);
	inline Column at(size_t idx) const
	{
		if (m_other_name_array != nullptr)
		{
			ATP_DATABASE_PRECOND(idx < m_indices.size());
			return Column(idx, m_other_name_array);
		}
		else
		{
			ATP_DATABASE_PRECOND(idx < m_name_array.size());
			return Column(m_name_array.at(idx));
		}
	}

private:
	// invariant: a column list is either an array of names, or it
	// has a reference to a different name array and contains a
	// a vector of the indices of the represented columns.
	std::vector<std::string> m_name_array;

	std::vector<size_t> m_indices;
	const std::vector<std::string>* m_other_name_array;
};


}  // namespace db
}  // namespace atp


