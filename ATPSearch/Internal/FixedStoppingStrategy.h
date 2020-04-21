#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a naive optimal stopping strategy

*/


#include "../ATPSearchAPI.h"
#include "../Interfaces/IStoppingStrategy.h"


namespace atp
{
namespace search
{


/**
\brief This is a naive, simple stopping strategy: always keep N
	successors in memory, and always just return the best one.

\details This strategy is naive because it pays no attention to cost
	and is hardly even a stopping strategy. However, its simplicity
	makes a great benchmark (if a complex strategy cannot beat this
	then it should be discarded).
*/
class ATP_SEARCH_API FixedStoppingStrategy :
	public IStoppingStrategy
{
public:
	/**
	\param N The number of successors that should be maintained in
		memory (bigger means more memory usage but closer to an
		optimal evaluation order).
	*/
	FixedStoppingStrategy(size_t N);

	void add(float benefit, float cost) override;
	bool is_stopped() const override;
	void max_removed() override;

private:
	const size_t m_N;  // target size
	size_t m_cur;  // current size
};


}  // namespace search
}  // namespace atp


