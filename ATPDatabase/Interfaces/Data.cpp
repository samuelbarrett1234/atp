/**
\file

\author Samuel Barrett

*/


#include "Data.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>


namespace atp
{
namespace db
{


std::shared_ptr<logic::IStatement> stmt_ref_to_ptr(
	const logic::IStatement& stmt)
{
	if (auto p_stmt = dynamic_cast<
		const logic::equational::Statement*>(&stmt))
	{
		return std::make_shared<logic::equational::Statement>(*p_stmt);
	}
	else
	{
		ATP_DATABASE_ASSERT(false && "bad statement type!");
		return nullptr;
	}
}


DValue::DValue(const logic::IStatement& x) :
	m_data(stmt_ref_to_ptr(x)),
	m_type(DType::STMT)
{
}


logic::StatementArrayPtr create_from_dvalues(
	const std::vector<DValue>& vals)
{
	// check type beforehand
	if (std::any_of(vals.begin(), vals.end(),
		boost::bind(&DValue::type, _1) != DType::STMT))
		return nullptr;

	std::vector<logic::StatementArrayPtr> singletons;
	singletons.reserve(vals.size());

	// bit inefficient :(
	for (const DValue& dv : vals)
	{
		ATP_DATABASE_ASSERT(dv.type() == DType::STMT);

		singletons.emplace_back(logic::from_statement(
			*dv.as_stmt()));
	}

	return logic::concat(singletons);
}


boost::variant2::variant<
	std::vector<int>,
	std::vector<size_t>,
	std::vector<float>,
	std::vector<std::string>,
	DArray::StmtArrRef> create_variant_from_dvalues(
		const std::vector<DValue>& vals,
		const logic::StatementArrayPtr& ptr)
{
	// precondition: they all have the same type
	const DType all_type = 
		vals.empty() ? DType::INT : vals.front().type();
	ATP_DATABASE_PRECOND(std::all_of(vals.begin(),
		vals.end(), boost::bind(&DValue::type, _1)
		== all_type));

	if (ptr != nullptr)
	{
		ATP_DATABASE_ASSERT(all_type == DType::STMT);

		return DArray::StmtArrRef(*ptr);
	}
	else
	{
		ATP_DATABASE_ASSERT(all_type != DType::STMT);

		switch (all_type)
		{
		case DType::INT:
			return std::vector<int>();
		case DType::UINT:
			return std::vector<size_t>();
		case DType::FLOAT:
			return std::vector<float>();
		case DType::STR:
			return std::vector<std::string>();
		default:
			ATP_DATABASE_ASSERT(false && "bad DType!");
			throw std::exception();
		}
	}
}


// for getting a DValue at a given index
struct DArrayValueAtVisitor
{
	const size_t i;
	DArrayValueAtVisitor(size_t i) :
		i(i)
	{ }

	template<typename T>
	DValue operator()(const T& arr)
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
	DValue operator()<DArray::StmtArrRef>(
		const DArray::StmtArrRef& arr)
	{
		if (auto p_arr = dynamic_cast<
			const logic::equational::StatementArray*>(&arr.ref))
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
	const size_t m;
	DArrayReserveVisitor(size_t m) :
		m(m)
	{ }

	template<typename T>
	void operator()(T& arr)
	{
		arr.reserve(m);
	}
	template<>
	void operator()<DArray::StmtArrRef>(
		DArray::StmtArrRef& arr)
	{
		// cannot reserve this kind of array
	}
};


// for reserving an amount of memory in the array
struct DArrayPushBackVisitor
{
	const DValue& val;
	DArrayPushBackVisitor(const DValue& val) :
		val(val)
	{ }

	template<typename T>
	void operator()(T& arr)
	{
		arr.push_back((typename T::value_type)val);
	}
	template<>
	void operator()<DArray::StmtArrRef>(
		DArray::StmtArrRef& _)
	{
		// don't implement - we handle this separately
	}
};


struct DArraySaveVisitor
{
	std::ostream& out;
	std::vector<size_t>* p_opt_elem_offs;

	DArraySaveVisitor(std::ostream& out,
		std::vector<size_t>* p_opt_elem_offs) :
		out(out), p_opt_elem_offs(p_opt_elem_offs)
	{ }

	template<typename T>
	void operator()(const T& arr)
	{
		using ValueType = typename T::value_type;

		// this version is for primitive data types
		std::ostream_iterator<ValueType> os_iter(out);
		std::copy(arr.begin(), arr.end(), os_iter);

		if (p_opt_elem_offs != nullptr)
		{
			// it's easy to compute the size for these data types

			p_opt_elem_offs->resize(arr.size());

			for (size_t i = 0; i < arr.size(); ++i)
			{
				p_opt_elem_offs->at(i) =
					sizeof(typename T::value_type) * (i + 1);
			}
		}
	}

	template<>
	void operator()(const std::vector<std::string>& arr)
	{
		boost::archive::binary_oarchive bo(out);

		if (p_opt_elem_offs != nullptr)
			p_opt_elem_offs->reserve(arr.size());

		const auto start = out.tellp();

		for (const auto& str : arr)
		{
			bo << str;

			if (p_opt_elem_offs != nullptr)
			{
				p_opt_elem_offs->push_back(out.tellp() - start);
			}
		}
	}

	template<>
	void operator()(const DArray::StmtArrRef& arr)
	{
		if (p_opt_elem_offs != nullptr)
			p_opt_elem_offs->reserve(arr.size());

		const auto start = out.tellp();

		if (auto p_arr = dynamic_cast<
			logic::equational::StatementArray*>(&arr.ref))
		{
			// need to use our own saving function! This is because
			// our version does not store array sizes (whereas the
			// standard StatementArray saving does in fact store the
			// size etc).
			// it is also less intrusive doing it by ourselves as
			// their implementation doesn't return the element
			// offsets/sizes

			for (size_t i = 0; i < p_arr->size(); ++i)
			{
				p_arr->my_at(i).save(out);

				if (p_opt_elem_offs != nullptr)
				{
					p_opt_elem_offs->push_back(out.tellp() - start);
				}
			}
		}
		else  // bad type!
		{
			ATP_DATABASE_ASSERT(false && "bad statement array type!");
		}
	}
};


DArray::DArray(const std::vector<DValue>& arr) :
	m_type(arr.empty() ? DType::INT : arr.front().type()),
	m_maybe_arr(create_from_dvalues(arr)),
	m_data(create_variant_from_dvalues(arr, m_maybe_arr))
{
	boost::variant2::visit(DArrayReserveVisitor(arr.size()), m_data);

	if (m_type != DType::STMT)
	{
		for (const DValue& dv : arr)
		{
			boost::variant2::visit(DArrayPushBackVisitor(dv),
				m_data);
		}
	}
}


DValue DArray::val_at(size_t idx) const
{
	return boost::variant2::visit(DArrayValueAtVisitor(idx), m_data);
}


void DArray::save_raw(std::ostream& out,
	std::vector<size_t>* p_opt_elem_offs) const
{
	boost::variant2::visit(DArraySaveVisitor(out, p_opt_elem_offs),
		m_data);
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
		// check that, at least, the names agree (even if the two
		// pointers are different).
		ATP_DATABASE_PRECOND(col.m_name_array == nullptr ||
			col.m_name_array->at(col.m_idx) ==
			m_other_name_array->at(col.m_idx));

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


size_t ColumnList::index_of(const Column& col) const
{
	if (col.has_name())
	{
		// find by name

		if (m_other_name_array != nullptr)
		{
			// we have an array of names, and they have an array of
			// names, so we need to make the two collaborate (going
			// via our array of indices, too).

			const auto iter = std::find(m_other_name_array->begin(),
				m_other_name_array->end(), col.name());

			const size_t idx = std::distance(
				m_other_name_array->begin(), iter);

			const auto idx_iter = std::find(m_indices.begin(),
				m_indices.end(), idx);

			return std::distance(m_indices.begin(), idx_iter);
		}
		else
		{
			// we have an array of names, and they have an array of
			// names, so we need to make the two collaborate.
			// However, we don't have an array of indices to worry
			// about.

			const auto iter = std::find(m_name_array.begin(),
				m_name_array.end(), col.name());

			return std::distance(
				m_name_array.begin(), iter);
		}
	}
	else
	{
		ATP_DATABASE_ASSERT(col.m_name_array == nullptr);
		ATP_DATABASE_ASSERT(col.m_name.empty());

		// this case is easy, as the column is just an index with no
		// name array associated with it.

		return col.m_idx;
	}
}


}  // namespace db
}  // namespace atp


