#pragma once


/*

StatementArray.h

Implementation of the IStatementArray interface for equational logic.
This class tries to be as lazy as possible, for example by sharing a
pointer to the original array. Note that this has the downside of
potentially keeping around more memory than necessary, but I think
this is worth the speedup.

*/


#include <memory>
#include <vector>
#include <iterator>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatementArray.h"
#include "Statement.h"


namespace atp
{
namespace logic
{
namespace equational
{


// Helper function for computing the number of elements in a slice.
// Precondition: start <= end and step > 0
// Postcondition: returns the size of the set
// { k >= 0 s.t. start + k * step < end }
ATP_LOGIC_API size_t compute_slice_size(size_t start, size_t end,
	size_t step);


class ATP_LOGIC_API StatementArray : public IStatementArray
{
public:
	// We DEFINITELY want to be using a vector of the derived type
	// Statement here, not a pointer to the base type
	// IStatement.
	typedef std::vector<Statement> ArrType;
	typedef std::shared_ptr<ArrType> ArrPtr;

public:
	class ATP_LOGIC_API iterator
	{
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef Statement value_type;
		typedef const Statement& reference;
		typedef const Statement* pointer;
		typedef size_t difference_type;

		inline iterator() :
			i(static_cast<size_t>(-1)),
			arr(nullptr)
		{ }
		inline iterator(const StatementArray* arr) :
			i(static_cast<size_t>(-1)),
			arr(arr)
		{ }
		inline iterator(const StatementArray* arr, size_t i) :
			i(i), arr(arr)
		{ }
		inline iterator(const iterator& other) :
			i(other.i),
			arr(other.arr)
		{ }
		inline iterator(iterator&& other) noexcept :
			i(other.i),
			arr(other.arr)
		{ other.arr = nullptr; }
		inline iterator& operator =(const iterator& other)
		{
			i = other.i;
			arr = other.arr;
			return *this;
		}
		inline iterator operator +(int j) const
		{
			if (!is_end_iterator())
			{
				if (j > 0)
					return iterator(arr,
						i + static_cast<size_t>(j));
				else
				{
					// can't go backwards before index 0
					ATP_LOGIC_PRECOND(static_cast<size_t>(-j)
						>= i);
					return iterator(arr,
						i - static_cast<size_t>(-j));

				}
			}
			else
			{
				if (j >= 0)
					return *this;
				else
					return iterator(arr,
						arr->size() + j);
			}
		}
		inline iterator operator -(int j) const
		{
			return *this + (-j);
		}
		inline difference_type operator - (const iterator& other) const
		{
			if (is_end_iterator() && other.is_end_iterator())
				return 0;
			else if (is_end_iterator() || i < other.i)
				return static_cast<size_t>(-1);
			else if (other.is_end_iterator())
				return (arr->size() - i);
			else
				return i - other.i;
		}
		inline iterator& operator += (int j)
		{
			*this = (*this + j);
			return *this;
		}
		inline iterator& operator -= (int j)
		{
			*this = (*this - j);
			return *this;
		}
		inline reference operator*() const
		{
			ATP_LOGIC_PRECOND(arr != nullptr);
			ATP_LOGIC_PRECOND(i < arr->size());
			return arr->my_at(i);
		}
		inline pointer operator->() const
		{
			ATP_LOGIC_PRECOND(arr != nullptr);
			ATP_LOGIC_PRECOND(i < arr->size());
			return &arr->my_at(i);
		}
		inline reference operator[](size_t j) const
		{
			return *(*this + (int)j);
		}
		inline iterator& operator++()
		{
			ATP_LOGIC_ASSERT(!is_end_iterator());
			++i;
			return *this;
		}
		inline iterator& operator--()
		{
			ATP_LOGIC_ASSERT(i > 0);
			if (is_end_iterator())
				i = arr->size() - 1;
			else
				--i;
			return *this;
		}
		inline iterator operator++(int)
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}
		inline iterator operator--(int)
		{
			iterator temp = *this;
			--(*this);
			return temp;
		}
		inline bool operator==(const iterator& iter) const
		{
			// warning: end iterators don't necessarily agree
			// on their index!
			// so if their indices are the same, or they are
			// both end iterators, then we treat them as equal.
			return ((i == iter.i) ||
				(is_end_iterator() && iter.is_end_iterator())
				&& arr == iter.arr);
		}
		inline bool operator!=(const iterator& iter) const
		{
			return !(*this == iter);
		}
		inline bool operator<(const iterator& iter) const
		{
			return ((i < iter.i && !iter.is_end_iterator()) ||
				(!is_end_iterator() && iter.is_end_iterator()));
		}
		inline bool operator<=(const iterator& iter) const
		{
			return (i <= iter.i
				|| (is_end_iterator() && iter.is_end_iterator()));
		}
		inline bool operator>(const iterator& iter) const
		{
			return !(*this <= iter);
		}
		inline bool operator>=(const iterator& iter) const
		{
			return !(*this < iter);
		}
		inline bool is_end_iterator() const
		{
			ATP_LOGIC_PRECOND(arr != nullptr);
			return i >= arr->size();
		}

	private:
		size_t i;
		const StatementArray* arr;
	};

public:
	// these functions return nullptr if the statement types given as
	// argument are not equational (thus this class isn't responsible
	// for handling them). Returning nullptr is NOT an error.
	static StatementArrayPtr try_from_stmt(const IStatement& stmt);
	static StatementArrayPtr try_concat(const IStatementArray& l,
		const IStatementArray& r);
	static StatementArrayPtr try_concat(
		const std::vector<StatementArrayPtr>& stmts);

public:
	// start/end/step are like the parameters given to slice() and
	// they follow the same preconditions.
	StatementArray(ArrPtr p_array = ArrPtr(),
		size_t start = 0, size_t end = static_cast<size_t>(-1),
		size_t step = 1);

	inline size_t size() const override
	{
		return compute_slice_size(m_start, m_stop, m_step);
	}
	const IStatement& at(size_t i) const override
	{
		ATP_LOGIC_PRECOND(i < size());
		ATP_LOGIC_ASSERT(m_array != nullptr);
		return static_cast<const IStatement&>(
			m_array->at(m_start + i * m_step));
	}
	const Statement& my_at(size_t i) const
	{
		return m_array->at(i);
	}
	inline iterator begin() const
	{
		return iterator(this, 0);
	}
	inline iterator end() const
	{
		return iterator(this);
	}
	StatementArrayPtr slice(size_t start, size_t end,
		size_t step = 1) const override;

private:
	ArrPtr m_array;

	// invariant: m_start >= 0, m_stop <= m_array->size(),
	// m_step >= 1
	const size_t m_start, m_stop, m_step;

	// invariant: the elements represented by this array are given by
	// the indices m_start + k * m_step for k >= 0 whenever that
	// index is < m_stop.
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


