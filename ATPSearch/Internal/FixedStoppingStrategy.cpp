/**
\file

\author Samuel Barrett
*/


#include "FixedStoppingStrategy.h"


namespace atp
{
namespace search
{


FixedStoppingStrategy::FixedStoppingStrategy(size_t N) :
	m_N(N), m_cur(0)
{
	ATP_SEARCH_PRECOND(m_N >= 1);
}


void FixedStoppingStrategy::add(float benefit, float cost)
{
	++m_cur;
}


bool FixedStoppingStrategy::is_stopped() const
{
	return (m_cur == m_N);
}


void FixedStoppingStrategy::max_removed()
{
	--m_cur;
}


}  // namespace search
}  // namespace atp


