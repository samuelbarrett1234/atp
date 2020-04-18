#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a `FreeVarIdSet` data structure for holding sets of
	free variable IDs.
*/


#include <algorithm>
#include <functional>
#include <boost/container/small_vector.hpp>
#include "../ATPLogicAPI.h"


namespace atp
{
namespace logic
{


/**
\brief This class represents a set of free variable IDs

\details Why not just use std::set<size_t>? Because this class takes
	advantage of the fact that free variable IDs are usually close
	together (dense), so we subtract the minimum ID from all of them,
	and then store it as a bitmap.

\note Don't need to export this class as it is fully inlined.
*/
class FreeVarIdSet
{
public:
	template<typename _ValTy,
		typename _PtrTy, typename _RefTy,
		typename _ParentTy>
	class _iterator
	{
		friend FreeVarIdSet;
	private:
		typedef _iterator<_ValTy, _PtrTy, _RefTy,
			_ParentTy> _my_iter_ty;
	public:
		// conforming to the standard iterator template interface

		typedef std::forward_iterator_tag iterator_category;
		typedef _ValTy value_type;
		typedef _RefTy reference;
		typedef _PtrTy pointer;
		typedef size_t difference_type;

		inline _iterator(_ParentTy parent,
			size_t idx) :
			m_parent(parent), m_idx(idx)
		{ }

		inline _iterator(const _my_iter_ty& other) :
			m_parent(other.m_parent),
			m_idx(other.m_idx)
		{ }

		inline _iterator(_my_iter_ty&& other) noexcept :
			m_parent(other.m_parent),
			m_idx(other.m_idx)
		{ }

		inline _my_iter_ty& operator= (const _my_iter_ty& other)
		{
			m_parent = other.m_parent;
			m_idx = other.m_idx;
			return *this;
		}

		inline _my_iter_ty&
			operator= (_my_iter_ty&& other) noexcept
		{
			m_parent = other.m_parent;
			m_idx = other.m_idx;
			return *this;
		}

		inline value_type operator*() const
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			return m_idx + m_parent->m_min_id;
		}

		pointer operator->() = delete;

		inline _my_iter_ty& operator++()
		{
			bring_forward();

			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());

			++m_idx;
			return *this;
		}

		inline _my_iter_ty operator++(int)
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}

		inline bool operator == (const _my_iter_ty& other) const
		{
			bring_forward();
			other.bring_forward();

			return (m_parent == other.m_parent &&
				m_idx == other.m_idx);
		}

		inline bool operator != (const _my_iter_ty& other) const
		{
			return !(*this == other);
		}

	private:
		void bring_forward() const
		{
			while (m_idx < m_parent->m_vec.size()
				&& !m_parent->m_vec[m_idx])
			{
				++m_idx;
			}
		}

	private:
		_ParentTy m_parent;
		mutable size_t m_idx;
	};

	typedef _iterator<size_t, const size_t*,
		const size_t&, const FreeVarIdSet*>
		iterator;
	typedef iterator const_iterator;

	typedef size_t value_type;

public:
	FreeVarIdSet() :
		m_min_id(0)
	{ }

	template<typename ContainerT>
	FreeVarIdSet(const ContainerT& cont) :
		FreeVarIdSet(cont.begin(), cont.end())
	{ }

	template<typename IterT>
	FreeVarIdSet(IterT _begin, IterT _end) :
		m_vec(std::distance(_begin, _end), false),
		m_min_id((_begin != _end) ? *_begin : 0)
	{
		for (auto iter = _begin; iter != _end; ++iter)
		{
			m_vec[*iter - m_min_id] = true;
		}
	}

	FreeVarIdSet(const FreeVarIdSet& other) :
		m_vec(other.m_vec),
		m_min_id(other.m_min_id)
	{ }
	FreeVarIdSet(FreeVarIdSet&& other) noexcept :
		m_vec(std::move(other.m_vec)),
		m_min_id(other.m_min_id)
	{ }
	FreeVarIdSet& operator= (const FreeVarIdSet& other)
	{
		m_vec = other.m_vec;
		m_min_id = other.m_min_id;
		return *this;
	}
	FreeVarIdSet& operator= (FreeVarIdSet&& other) noexcept
	{
		m_vec = std::move(other.m_vec);
		m_min_id = other.m_min_id;
		return *this;
	}

	inline bool operator == (const FreeVarIdSet& other) const
	{
		return subset(other) && other.subset(*this);
	}
	inline bool operator != (const FreeVarIdSet& other) const
	{
		return !(*this == other);
	}

	inline void insert(size_t id)
	{
		if (m_vec.empty())
		{
			m_min_id = id;
			m_vec.resize(1, true);
		}
		// three other cases (in two of these we have to resize
		// m_vec)
		else if (id < m_min_id)
		{
			// add elements to the beginning
			const size_t extend_size = m_min_id - id;
			m_min_id = id;
			// extend with many elements at the back
			m_vec.resize(m_vec.size() + extend_size, false);
			// swap those new elements to the front
			std::rotate(m_vec.rbegin(), m_vec.rbegin() + extend_size,
				m_vec.rend());
			m_vec[0] = true;  // set minimum to be true
		}
		else if (id >= m_min_id + m_vec.size())
		{
			const size_t extend_size =
				id - m_min_id - m_vec.size() + 1;
			m_vec.resize(m_vec.size() + extend_size, false);
			m_vec.back() = true;
		}
		else
		{
			m_vec[id - m_min_id] = true;
		}
	}

	inline bool contains(size_t id) const
	{
		if (id < m_min_id)
			return false;
		else if (id >= m_min_id + m_vec.size())
			return false;
		else return m_vec[id - m_min_id];
	}

	inline void erase(size_t id)
	{
		if (id >= m_min_id && id < m_vec.size() + m_min_id)
		{
			m_vec[id - m_min_id] = false;
		}
	}
	inline void erase(iterator iter)
	{
		iter.bring_forward();
		m_vec[iter.m_idx] = false;
	}

	/**
	\brief Returns true if every ID in this set, is also present in
		the `other` set.

	\note Of course if this set is empty then it will be a subset of
		any other set.
	*/
	inline bool subset(const FreeVarIdSet& other) const
	{
		for (size_t i = 0; i < size(); ++i)
		{
			if (m_vec[i] && !other.contains(m_min_id + i))
				return false;
		}
		return true;
	}

	/**
	\brief Remove all of the IDs i such that pred(i) is true

	\tparam PredT must be of type size_t -> bool
	*/
	template<typename PredT>
	inline void remove_if(PredT pred)
	{
		static_assert(std::is_convertible<PredT,
			std::function<bool(size_t)>>::value,
			"PredT should be of type size_t -> bool");

		for (size_t i = 0; i < m_vec.size(); ++i)
		{
			if (m_vec[i] && pred(i + m_min_id))
				m_vec[i] = false;
		}
	}

	inline size_t size() const
	{
		return std::count(m_vec.begin(), m_vec.end(), true);
	}
	inline size_t max() const
	{
		ATP_LOGIC_PRECOND(!empty());
		auto iter = std::find(m_vec.rbegin(), m_vec.rend(),
			true);
		ATP_LOGIC_ASSERT(iter != m_vec.rend());
		return m_min_id + std::distance(m_vec.begin(),
			iter.base()) - 1;
	}
	inline bool empty() const
	{
		return std::none_of(m_vec.begin(), m_vec.end(),
			[](bool b) { return b; });
	}

	inline iterator begin() const
	{
		return iterator(this, 0);
	}
	inline iterator end() const
	{
		return iterator(this, m_vec.size());
	}

private:
	// invariant: i is an ID in this set if and only if
	// i >=  m_min_id and m_vec[i - m_min_id] is true
	boost::container::small_vector<bool, 5> m_vec;
	size_t m_min_id;
};


}  // namespace logic
}  // namespace atp


