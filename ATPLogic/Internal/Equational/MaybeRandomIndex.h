#pragma once


/**
\file

\author Samuel Barrett

\brief Iterate over a data structure either in order or randomly
*/


#include <algorithm>
#include <utility>
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
		m_idx(0),
		m_idxs(randomised ? size : 0)
	{
		// initialise the array
		for (size_t i = 0; i < m_size && m_randomised; ++i)
		{
			m_idxs[i] = i;
		}

		// Fisher Yates shuffle (we will repeatedly modify the random
		// input to generate a pseudo-random sequence from a single
		// random number input).
		for (size_t i = m_size - 1; i > 0 && m_randomised
			&& m_size > 0  /* `i` fails when size==0 */; --i)
		{
			const size_t j = random_input % i;

			// generate next random number in the sequence
			// (overflow is okay here)
			// https://en.wikipedia.org/wiki/Linear_congruential_generator
			random_input = (1664525 * random_input + 1013904223) % 0xffffffff;

			std::swap(m_idxs[i], m_idxs[j]);
		}
	}

	/**
	\brief Determine if this is an end index
	*/
	inline bool is_end() const
	{
		return m_idx == m_size;
	}

	/**
	\brief Get the index represented by this class
	*/
	inline size_t get() const
	{
		ATP_LOGIC_PRECOND(!is_end());
		
		return m_randomised ? m_idxs[m_idx] : m_idx;
	}

	/**
	\brief Advance the index to the next one
	*/
	inline void advance()
	{
		ATP_LOGIC_PRECOND(!is_end());
		++m_idx;
	}

private:
	const size_t m_size;
	const bool m_randomised;
	size_t m_idx;
	std::vector<size_t> m_idxs;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


