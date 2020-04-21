#include "BasicStoppingStrategy.h"
#include <cmath>
#include <boost/math/distributions/normal.hpp>


namespace atp
{
namespace search
{


// compute mean and variance from the sum of the data and the sum of
// the squares of the data (returns <mean, variance>).
std::pair<float, float> compute_mean_variance(float sum,
	float sum_sq, size_t n);


BasicStoppingStrategy::BasicStoppingStrategy(size_t num_initial_fill,
	float lambda, float alpha) :
	m_num_initial_left(num_initial_fill),
	m_lambda(lambda),
	m_alpha(alpha),
	m_sum_benefits(0), m_sum_sq_benefits(0),
	m_sum_log_costs(0), m_sum_sq_log_costs(0),
	m_data_size(0), m_stopped(false)
{
	ATP_SEARCH_PRECOND(m_lambda > 0.0f);
	ATP_SEARCH_PRECOND(m_alpha > 0.0f);
	ATP_SEARCH_PRECOND(m_alpha < 1.0f);
	ATP_SEARCH_PRECOND(m_num_initial_left > 1);
}


void BasicStoppingStrategy::add(float benefit, float cost)
{
	ATP_SEARCH_PRECOND(!is_stopped());
	ATP_SEARCH_PRECOND(cost > 0.0f);  // lognormal cannot be 0

	const float log_cost = std::log(cost);

	m_remaining_benefits.push(benefit);

	m_sum_benefits += benefit;
	m_sum_sq_benefits += benefit * benefit;

	m_sum_log_costs += log_cost;
	m_sum_sq_log_costs += log_cost * log_cost;

	++m_data_size;

	if (m_num_initial_left > 0)
		--m_num_initial_left;

	compute_stopped();
}


void BasicStoppingStrategy::compute_stopped()
{
	ATP_SEARCH_ASSERT(m_sum_sq_benefits > 0.0f);
	ATP_SEARCH_ASSERT(m_sum_sq_log_costs > 0.0f);
	ATP_SEARCH_ASSERT(m_data_size > 0);

	if (m_num_initial_left > 0)
	{
		// still need to collect initial data
		m_stopped = false;
		return;
	} 

	if (m_remaining_benefits.empty())
	{
		// need to extract more data
		m_stopped = false;
		return;
	}

	// compute means and variances

	auto [benefit_mean, benefit_var] = compute_mean_variance(
		m_sum_benefits, m_sum_sq_benefits, m_data_size);
	auto [log_cost_mean, log_cost_var] = compute_mean_variance(
		m_sum_log_costs, m_sum_sq_log_costs, m_data_size);

	// compute distribution of lambda*Benefit-log(Cost)

	boost::math::normal_distribution<float> dist(
		m_lambda * benefit_mean - log_cost_mean,
		std::sqrt(m_lambda * m_lambda * benefit_var + log_cost_var));

	// compute threshold

	const float thresh = m_lambda * m_remaining_benefits.top()
		+ m_sum_log_costs;

	/*
	Now, the event that dist > thresh is precisely the event that
	lambda(Benefit - best_benefit_seen_so_far) > Cost + sum_of_log_costs_seen_so_far
	*/

	const float p = boost::math::cdf(boost::math::complement(
		dist, thresh));

	// does this probability exceed the significance threshold?
	m_stopped = (p > m_alpha);
}


void BasicStoppingStrategy::max_removed()
{
	ATP_SEARCH_PRECOND(is_stopped());
	ATP_SEARCH_ASSERT(!m_remaining_benefits.empty());

	m_remaining_benefits.pop();

	compute_stopped();
}


std::pair<float, float> compute_mean_variance(float sum,
	float sum_sq, size_t n)
{
	ATP_SEARCH_PRECOND(n > 0);

	float mean = sum / n;
	float var = sum_sq / n - mean * mean;
	
	return std::make_pair(mean, var);
}


}  // namespace search
}  // namespace atp

