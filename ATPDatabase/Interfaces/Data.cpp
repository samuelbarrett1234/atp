/**
\file

\author Samuel Barrett

*/


#include "Data.h"
#include <algorithm>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>


namespace atp
{
namespace db
{


// for getting a DValue at a given index
struct DArrayValueAtVisitor
{
	template<typename T>
	DValue operator()(const T& arr, size_t i)
	{
		return DValue(arr.at(i));
	}

	/*
	Handle this differently, because we need to guarantee that: if
	the array is destroyed, the value we extract from it is still
	available. This involves copying an IStatement and putting it
	into a shared_ptr. However, statements don't support a copy
	function, so we have to specialise this to the logic type! :(
	*/
	template<>
	DValue operator()<const logic::IStatementArray&>(
		const logic::IStatementArray& arr, size_t i)
	{
		if (auto p_arr = dynamic_cast<
			const logic::equational::StatementArray*>(&arr))
		{
			return DValue(std::make_shared<logic::equational::Statement>(
				p_arr->my_at(i)));
		}
		else
		{
			ATP_DATABASE_ASSERT(false && "Bad statement array type!");
			throw std::exception();
		}
	}
};


// for reserving an amount of memory in the array
struct DArrayReserveVisitor
{
	template<typename T>
	void operator()(T& arr, size_t m)
	{
		arr.reserve(m);
	}
	template<>
	void operator()<logic::IStatementArray>(
		logic::IStatementArray& arr, size_t m)
	{
		// cannot reserve this kind of array
	}
};


// for reserving an amount of memory in the array
struct DArrayPushBackVisitor
{
	template<typename T>
	void operator()(T& arr, const DValue& val)
	{
		arr.push_back((typename T::value_type)val);
	}
	template<>
	void operator()<logic::IStatementArray>(
		logic::IStatementArray& _, const DValue& val)
	{
		// don't implement - we handle this separately
	}
};


DArray::DArray(const std::vector<DValue>& arr) :
	m_type(arr.empty() ? DType::INT : arr.front().type())
{
	ATP_DATABASE_PRECOND(std::all_of(arr.begin(),
		arr.end(), [this](const DValue& dv)
		{ return dv.type() == m_type; }));

	boost::variant2::visit(DArrayReserveVisitor(), m_data,
		arr.size());

	if (m_type != DType::STMT)
	{
		for (const DValue& dv : arr)
		{
			boost::variant2::visit(DArrayPushBackVisitor(),
				m_data, dv);
		}
	}
	else
	{
		// bit inefficient :(

		std::vector<logic::StatementArrayPtr> singletons;
		singletons.reserve(arr.size());
		for (const DValue& dv : arr)
		{
			singletons.emplace_back(logic::from_statement(
				*dv.as_stmt()));
		}
		m_maybe_arr = logic::concat(singletons);
		m_data = *m_maybe_arr;
	}
}


DValue DArray::val_at(size_t idx) const
{
	return boost::variant2::visit(DArrayValueAtVisitor(), m_data);
}


bool ColumnList::contains(const Column& col) const
{
	if (m_other_name_array == nullptr)
	{
		ATP_DATABASE_PRECOND(col.has_name());
		auto iter = std::find(m_name_array.begin(),
			m_name_array.end(), col.name());
		return (iter != m_name_array.end());
	}
	else
	{
		size_t idx;
		if (col.has_index())
		{
			idx = col.index();
		}
		else
		{
			ATP_DATABASE_ASSERT(col.has_name());

			idx = std::distance(m_other_name_array->begin(),
				std::find(m_other_name_array->begin(),
					m_other_name_array->end(), col.name()));
		}
		auto iter = std::find(m_indices.begin(),
			m_indices.end(), idx);
		return (iter != m_indices.end());
	}
}


void ColumnList::insert(const Column& col)
{
	if (col.has_index() && m_other_name_array != nullptr)
	{
		const size_t idx = col.index();
		ATP_DATABASE_PRECOND(idx < m_other_name_array->size());
		m_indices.push_back(idx);
	}
	else
	{
		ATP_DATABASE_PRECOND(col.has_name());

		if (m_other_name_array != nullptr)
		{
			auto iter = std::find(m_other_name_array->begin(),
				m_other_name_array->end(), col.name());

			ATP_DATABASE_PRECOND(iter != m_other_name_array->end());

			m_indices.push_back(std::distance(
				m_other_name_array->begin(), iter));
		}
		else
		{
			m_name_array.push_back(col.name());
		}
	}
}


}  // namespace db
}  // namespace atp


