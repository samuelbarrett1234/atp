#pragma once


/**
\file

\author Samuel Barrett

\brief Iterate over a data structure either in order or randomly
*/


#include <algorithm>
#include <boost/math/special_functions/prime.hpp>
#include "../../ATPLogicAPI.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief Some successor iterators either want to iterate over their
	results in order, or randomly. This class allows them to do
	both in the same way.

\details When randomised = false, this just behaves like a size_t
	index. When randomised = true, it iterates over the given input
	range in a random order.

\note The random iteration is done via the algebraic notion of
	primitive roots modulo n. See:
	https://stackoverflow.com/questions/33097292/c-iterate-vector-randomly
*/
class MaybeRandomIndex
{
public:
	/**
	\param size The size of the array this index will be looping over

	\param randomised A boolean flag indicating whether or not we
		should iterate over the size in a random order

	\param random_input A uniform random number input (to be used by
		this class, to iterate randomly if necessary). This class
		does not generate its own random numbers and requires this
		input first.
	*/
	MaybeRandomIndex(size_t size,
		bool randomised, size_t random_input) :
		m_size(size),
		m_randomised(randomised),
		m_idx((size > 1) ? 1 : 0),
		m_num_advances(0),
		m_prime([size]() {
		// compute the smallest prime which is >= m_size
		// (since m_prime is const we have to do this in a lambda)
		size_t n = 1, p = 3;
		// n = 0 corresponds to 2 (but we start from 3)
		while ((p = (size_t)boost::math::prime((unsigned int)n)) < size)
		{
			++n;

			// hopefully should never trigger this, although it is
			// actually not a guarantee (it is okay for less than
			// 10^5 but might become a problem after that)
			ATP_LOGIC_ASSERT(n < boost::math::max_prime);
		}
		return p;
			}()),
		m_some_random_number(randomised ?
			0 : (2 + random_input % (m_prime - 2)))
	{
		ATP_LOGIC_ASSERT(m_prime >= m_size);
		ATP_LOGIC_ASSERT(m_some_random_number >= 2);
		ATP_LOGIC_ASSERT(m_some_random_number < m_prime);
		if (m_randomised && m_size > 1)
			advance();  // initialises m_idx properly
	}

	/**
	\brief Determine if this is an end index
	*/
	inline bool is_end() const
	{
		return m_num_advances == m_size;
	}

	/**
	\brief Get the index represented by this class
	*/
	inline size_t get() const
	{
		ATP_LOGIC_PRECOND(!is_end());
		ATP_LOGIC_ASSERT(!m_randomised || m_idx - 1 < m_size);
		ATP_LOGIC_ASSERT(!m_randomised || m_idx >= 1);
		
		return m_randomised ? m_idx : m_num_advances;
	}

	/**
	\brief Advance the index to the next one
	*/
	inline void advance()
	{
		ATP_LOGIC_PRECOND(!is_end());
		++m_num_advances;
		if (m_randomised)
		{
			do
			{
				m_idx = (m_idx * m_some_random_number) % m_prime;
			} while (m_idx - 1 >= m_size);
		}
	}

private:
	const size_t m_size;
	const bool m_randomised;
	size_t m_num_advances;

	// if randomised

	const size_t m_prime;  // the largest odd prime >= m_size.
	// some random number between 2 and m_prime inclusive
	size_t m_idx, m_some_random_number;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


