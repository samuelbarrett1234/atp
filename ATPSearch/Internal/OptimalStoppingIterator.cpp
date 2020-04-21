/**
\file

\author Samuel Barrett
*/


#include "OptimalStoppingIterator.h"
#include <boost/timer/timer.hpp>


namespace atp
{
namespace search
{


OptimalStoppingIterator::OptimalStoppingIterator(
	logic::PfStateSuccIterPtr child,
	StoppingStrategyPtr stopping_strategy,
	HeuristicPtr benefit_heuristic) :
	m_child(std::move(child)),
	m_stopping_strategy(std::move(stopping_strategy)),
	m_benefit_heuristic(std::move(benefit_heuristic))
{
	// prepare the iterator
	forward();
}


bool OptimalStoppingIterator::valid() const
{
	return m_states.empty();
}


logic::ProofStatePtr OptimalStoppingIterator::get() const
{
	ATP_SEARCH_PRECOND(valid());
	ATP_SEARCH_ASSERT(!m_states.empty());
	return m_states.top()->get();
}


void OptimalStoppingIterator::advance()
{
	ATP_SEARCH_PRECOND(valid());
	ATP_SEARCH_ASSERT(!m_states.empty());
	ATP_SEARCH_ASSERT(!m_child->valid() || m_stopping_strategy->is_stopped());

	m_states.pop();

	// if the child is invalid, we totally ignore m_stopping_strategy
	if (m_child->valid())
		m_stopping_strategy->max_removed();

	// restore the state
	forward();
}


size_t OptimalStoppingIterator::size() const
{
	return m_states.size() * m_states.top()->size();
}


void OptimalStoppingIterator::forward()
{
	ATP_SEARCH_ASSERT(m_stopping_strategy != nullptr);
	ATP_SEARCH_ASSERT(m_child != nullptr);

	boost::timer::cpu_timer timer;
	timer.stop();  // timer starts by default when constructed

	while (!m_stopping_strategy->is_stopped() && m_child->valid())
	{
		timer.start();

		auto p_new_state = m_child->get();
		m_child->advance();

		timer.stop();

		// the extraction time is our cost
		const auto time = timer.elapsed();
		const float time_s = static_cast<float>(time.user
			+ time.system) * 1.0e-9f;  // convert to seconds

		const float benefit = m_benefit_heuristic->predict(
			p_new_state);

		// register cost and benefit
		m_stopping_strategy->add(benefit, time_s);

		// add the state to our priority queue
		m_states.push(std::make_pair(benefit,
			std::move(p_new_state)));
	}
}


}  // namespace search
}  // namespace atp


