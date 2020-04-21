#pragma once


#include <memory>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/**
\interface IOptimalStoppingStrategy

\brief A strategy for stopping when we've seen elements with a high
	"benefit", but every time we don't stop we incur a "cost".

\details Given an infinite sequence of random costs and benefits, but
	without knowing their distribution ahead of time, decide when to
	stop and return the best or when to carry on obtaining more. Note
	that this strategy will work under the assumption that there are
	infinitely many elements in the sequence - of course, this is not
	the case in practice, however as soon as we reach the end of the
	sequence, the stopping strategy is trivial: just return the
	elements in nonincreasing order.
*/
class ATP_SEARCH_API IOptimalStoppingStrategy
{
public:
	virtual ~IOptimalStoppingStrategy() = default;

	/**
	\brief Register a newly extracted data point

	\pre !is_stopped() - if we were stopped, one should not have
		extracted a new data point!
	*/
	virtual void add(float benefit, float cost) = 0;

	/**
	\brief Determine if the strategy is saying you should stop and
		return the maximum.

	\returns True iff stopped.

	\post If the strategy has been given no data (like when it is
		just initialised), is_stopped() returns false, to allow for
		data to be extracted.
	*/
	virtual bool is_stopped() const = 0;

	/**
	\brief Register the fact that we have returned the highest
		benefit element.

	\pre is_stopped() - if we were not stopped, we shouldn't be
		returning elements and should instead be extracting new ones!
	*/
	virtual void max_removed() = 0;
};


typedef std::unique_ptr<
	IOptimalStoppingStrategy> OptimalStoppingStrategyPtr;


}  // namespace search
}  // namespace atp


