#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a datastructure for mapping free variable IDs to
	arbitrary values.
*/


#include <algorithm>
#include <functional>
#include <type_traits>
#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>
#include "../ATPLogicAPI.h"


namespace atp
{
namespace logic
{


/**
\brief This class represents a mapping from free variable IDs to some
	value type.

\details This has better performance than std::map<size_t, _ValTy>
	because we are leveraging the fact that free variable IDs are
	"dense", so we can use them as the indices of an array.

\note We don't need to export this class because it is fully inlined.
*/
template<typename _ValTy>
class FreeVarMap
{
public:
	template<typename _ValTy,
		typename _PtrTy, typename _RefTy,
		typename _ParentTy>
	class _iterator
	{
		friend FreeVarMap;
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

		inline std::add_const_t<reference> operator*() const
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			ATP_LOGIC_ASSERT(m_parent->m_vec[m_idx].has_value());
			return *m_parent->m_vec[m_idx];
		}

		inline std::enable_if_t<!std::is_const_v<reference>
			&& !std::is_const_v<_ParentTy>,
			reference> operator*()
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			ATP_LOGIC_ASSERT(m_parent->m_vec[m_idx].has_value());
			return *m_parent->m_vec[m_idx];
		}

		inline size_t first() const
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			ATP_LOGIC_ASSERT(m_parent->m_vec[m_idx].has_value());
			return m_idx + m_parent->m_min_id;
		}

		inline std::add_const_t<reference> second() const
		{
			return *(*this);
		}

		inline std::enable_if_t<!std::is_const_v<reference>
			&& !std::is_const_v<_ParentTy>,
			reference> second()
		{
			return *(*this);
		}

		inline typename std::add_const_t<pointer> operator->() const
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			ATP_LOGIC_ASSERT(m_parent->m_vec[m_idx].has_value());
			return *m_parent->m_vec[m_idx].get_ptr();
		}

		inline std::enable_if_t<!std::is_const_v<pointer>
			&& !std::is_const_v<_ParentTy>,
			pointer> operator->()
		{
			bring_forward();
			ATP_LOGIC_PRECOND(m_idx < m_parent->m_vec.size());
			ATP_LOGIC_ASSERT(m_parent->m_vec[m_idx].has_value());
			return *m_parent->m_vec[m_idx].get_ptr();
		}

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
				&& !m_parent->m_vec[m_idx].has_value())
			{
				++m_idx;
			}
		}

	private:
		_ParentTy m_parent;
		mutable size_t m_idx;
	};

	typedef _ValTy value_type;
	typedef _ValTy* pointer_type;
	typedef const _ValTy* const_pointer_type;
	typedef std::add_lvalue_reference_t<_ValTy
		> lvalue_reference;
	typedef std::add_lvalue_reference_t<std::add_const_t<_ValTy
		>> const_lvalue_reference;
	typedef std::add_rvalue_reference_t<_ValTy
		> rvalue_reference;

	typedef _iterator<value_type, pointer_type,
		lvalue_reference, FreeVarMap*> iterator;
	typedef _iterator<value_type, const_pointer_type,
		const_lvalue_reference, const FreeVarMap*> const_iterator;

public:
	FreeVarMap() :
		m_min_id(0)
	{ }

	FreeVarMap(const FreeVarMap& other) :
		m_vec(other.m_vec),
		m_min_id(other.m_min_id)
	{ }
	FreeVarMap(FreeVarMap&& other) noexcept :
		m_vec(std::move(other.m_vec)),
		m_min_id(other.m_min_id)
	{ }
	FreeVarMap& operator= (const FreeVarMap& other)
	{
		m_vec = other.m_vec;
		m_min_id = other.m_min_id;
		return *this;
	}
	FreeVarMap& operator= (FreeVarMap&& other) noexcept
	{
		m_vec = std::move(other.m_vec);
		m_min_id = other.m_min_id;
		return *this;
	}

	inline bool operator == (const FreeVarMap& other) const = delete;
	inline bool operator != (const FreeVarMap& other) const = delete;
	
	template<typename _ValTy2>  // allow lvalue and rvalue references
	inline void insert(size_t id, _ValTy2 val)
	{
		static_assert(std::is_convertible_v<_ValTy2, value_type>,
			"_ValTy2 type needs to be convertible to value_type");

		const size_t idx = ensure_size(id);

		ATP_LOGIC_ASSERT(idx < m_vec.size());

		m_vec[idx] = val;
	}

	inline void erase(size_t id)
	{
		if (id >= m_min_id && id < m_vec.size() + m_min_id)
		{
			m_vec[id - m_min_id] = boost::none;
		}
	}
	inline void erase(iterator iter)
	{
		iter.bring_forward();
		m_vec[iter.m_idx] = boost::none;
	}
	inline void erase(const_iterator iter)
	{
		iter.bring_forward();
		m_vec[iter.m_idx] = boost::none;
	}

	inline bool contains(size_t id) const
	{
		if (id >= m_min_id && id < m_vec.size() + m_min_id)
		{
			return m_vec[id - m_min_id].has_value();
		}
		else return false;
	}

	inline iterator find(size_t id)
	{
		if (contains(id))
			return iterator(this, id - m_min_id);
		else
			return end();
	}
	inline const_iterator find(size_t id) const
	{
		if (contains(id))
			return const_iterator(this, id - m_min_id);
		else
			return end();
	}

	inline const_lvalue_reference at(size_t id) const
	{
		ATP_LOGIC_PRECOND(id >= m_min_id &&
			id < m_vec.size() + m_min_id);
		return *m_vec[id - m_min_id];
	}

	// only enable this version if the reference isn't const
	inline std::enable_if_t<!std::is_const<lvalue_reference>::value,
		lvalue_reference> at(size_t id)
	{
		ATP_LOGIC_PRECOND(id >= m_min_id &&
			id < m_vec.size() + m_min_id);
		return *m_vec[id - m_min_id];
	}

	inline size_t size() const
	{
		return std::count_if(m_vec.begin(), m_vec.end(),
			[](const boost::optional<_ValTy>& b)
			{ return b.has_value(); });
	}
	inline bool empty() const
	{
		return std::none_of(m_vec.begin(), m_vec.end(),
			[](const boost::optional<_ValTy>& b)
			{ return b.has_value(); });
	}

	void clear()
	{
		for (auto& opt : m_vec)
			opt = boost::none;
		ATP_LOGIC_ASSERT(empty());
	}

	inline iterator begin()
	{
		return iterator(this, 0);
	}
	inline iterator end()
	{
		return iterator(this, m_vec.size());
	}
	inline const_iterator begin() const
	{
		return const_iterator(this, 0);
	}
	inline const_iterator end() const
	{
		return const_iterator(this, m_vec.size());
	}

private:
	/**
	\brief Ensure the array has the size to accomodate this ID

	\returns The index of the position in the array representing this
		ID.
	*/
	inline size_t ensure_size(size_t id)
	{
		if (m_vec.empty())
		{
			m_min_id = id;
			m_vec.push_back(boost::none);
			return 0;  // front
		}
		// three other cases (in two of these we have to resize
		// m_vec)
		else if (id < m_min_id)
		{
			// add elements to the beginning
			const size_t extend_size = m_min_id - id;
			m_min_id = id;
			// extend with many elements at the back
			m_vec.resize(m_vec.size() + extend_size, boost::none);
			// swap those new elements to the front
			std::rotate(m_vec.rbegin(), m_vec.rbegin() + extend_size,
				m_vec.rend());
			return 0;  // front
		}
		else if (id >= m_min_id + m_vec.size())
		{
			const size_t extend_size =
				id - m_min_id - m_vec.size() + 1;
			m_vec.resize(m_vec.size() + extend_size, boost::none);
			return m_vec.size() - 1;  // back
		}
		else
		{
			return id - m_min_id;
		}
	}

private:
	// invariant: i is an ID is mapped to if and only if
	// i >=  m_min_id and m_vec[i - m_min_id] is has a value
	boost::container::small_vector<
		boost::optional<_ValTy>, 5> m_vec;
	size_t m_min_id;
};


}  // namespace logic
}  // namespace atp


