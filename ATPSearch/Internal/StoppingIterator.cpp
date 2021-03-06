/**
\file

\author Samuel Barrett
*/


#include "StoppingIterator.h"
#include <boost/timer/timer.hpp>


namespace atp
{
namespace search
{


StoppingIterator::StoppingIterator(
	logic::PfStateSuccIterPtr child,
	StoppingStrategyPtr stopping_strategy,
	HeuristicPtr benefit_heuristic) :
	m_child(std::move(child)),
	m_stopping_strategy(std::move(stopping_strategy)),
	m_benefit_heuristic(std::move(benefit_heuristic))
{
	ATP_SEARCH_PRECOND(m_child != nullptr);
	ATP_SEARCH_PRECOND(m_stopping_strategy != nullptr);
	ATP_SEARCH_PRECOND(m_benefit_heuristic != nullptr);

	// prepare the iterator
	forward();
}


bool StoppingIterator::valid() const
{
	const bool valid = !m_states.empty();
	
	// all successors must go through m_states before coming out of
	// m_child, thus it cannot be the case that m_states is empty
	// but m_child is valid
	ATP_SEARCH_ASSERT(valid || !m_child->valid());

	return valid;
}


logic::ProofStatePtr StoppingIterator::get() const
{
	ATP_SEARCH_PRECOND(valid());
	ATP_SEARCH_ASSERT(!m_states.empty());
	return m_states.top().second;
}


void StoppingIterator::advance()
{
	ATP_SEARCH_PRECOND(valid());
	ATP_SEARCH_ASSERT(!m_states.empty());
	ATP_SEARCH_ASSERT(!m_child->valid() ||
		m_stopping_strategy->is_stopped());

	// the top element of m_states represents our current state, so
	// when we advance we chuck out or (last) current state and
	// obtain a new one
	m_states.pop();

	// if the child is invalid, we totally ignore m_stopping_strategy
	// so don't bother notifying it of the new child
	if (m_child->valid())
		m_stopping_strategy->max_removed();

	// restore the state
	forward();
}


size_t StoppingIterator::size() const
{
	// this is quite a rough heuristic, but it'll do
	if (m_child->valid())
		return m_states.size() * m_child->size();
	else
		return m_states.size();
}


void StoppingIterator::forward()
{
	ATP_SEARCH_ASSERT(m_stopping_strategy != nullptr);
	ATP_SEARCH_ASSERT(m_child != nullptr);

	boost::timer::cpu_timer timer;
	timer.stop();  // timer starts by default when constructed

	// we can only generate new successors if m_child is valid
	// furthermore, if this is the case, we absolutely need new
	// successors if m_states is empty, otherwise we follow the
	// orders of the stopping strategy.
	while ((m_states.empty() || !m_stopping_strategy->is_stopped())
		&& m_child->valid())
	{
		timer.start();

		// generate next successor (make sure to call get/advance in
		// this order)
		auto p_new_state = m_child->get();
		m_child->advance();

		// compute benefit (and include this computation as part of
		// the cost timing, as this can also be expensive).
		const float benefit = m_benefit_heuristic->predict(
			p_new_state);

		timer.stop();

		// the extraction time is our cost
		const auto time = timer.elapsed();
		const float time_s = static_cast<float>(time.user
			+ time.system) * 1.0e-9f;  // convert to seconds

		// register cost and benefit
		m_stopping_strategy->add(benefit, time_s
			+ 1.0e-6f /* just so it doesn't complain about 0 cost */
		);

		// add the state to our priority queue
		m_states.push(std::make_pair(benefit,
			std::move(p_new_state)));
	}
}


}  // namespace search
}  // namespace atp


