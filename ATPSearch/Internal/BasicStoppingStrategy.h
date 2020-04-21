#pragma once


/**
\file

\author Samuel Barrett

\brief A simple stopping strategy using some basic statistics

*/


#include <queue>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IStoppingStrategy.h"


namespace atp
{
namespace search
{


/**
\brief This strategy uses a simple cost vs benefit tradeoff

\details The benefits are modelled as normal, and the costs are
	modelled as lognormal. At each timestep, we compute the
	tradeoff between (i) a potential increase in the highest benefit
	we have seen so far vs (ii) an increase in the total cost.
*/
class ATP_SEARCH_API BasicStoppingStrategy :
	public IStoppingStrategy
{
public:
	/**
	\param num_initial_fill The number of elements to extract right
	    at the start, so we can get decent estimates of the
		distribution parameters. Must be > 1.

	\param lambda The cost multiplier (lower means we are willing to
		accept more cost to achieve a certain benefit). Must be > 0.

	\param alpha The probability significance threshold. If the
		probability that one more evaluation would be worth it is
		greater than alpha, then we continue, else we stop. Must be
		strictly between 0 and 1.

	\pre num_initial_fill > 1, lambda > 0, 0 < alpha < 1
	*/
	BasicStoppingStrategy(size_t num_initial_fill,
		float lambda, float alpha);

	void add(float benefit, float cost) override;
	inline bool is_stopped() const override
	{
		return m_stopped;
	}
	void max_removed() override;

private:
	void compute_stopped();

private:
	// hyperparameters
	const float m_lambda, m_alpha;

	// cache the return result of is_stopped
	bool m_stopped;

	// the sum and sum of squares of the benefits/costs
	float m_sum_benefits,
		m_sum_sq_benefits,
		m_sum_log_costs,
		m_sum_sq_log_costs;
	size_t m_data_size;

	// the number of initial elements left for us to extract, in
	// order to get a good initial estimate of our distribution
	// parameters
	size_t m_num_initial_left;

	// the benefit values remaining
	std::priority_queue<float> m_remaining_benefits;
};


}  // namespace search
}  // namespace atp


