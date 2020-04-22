/**
\file

\author Samuel Barrett

*/


#include "IteratorManager.h"
#include "StoppingIterator.h"
#include "FixedStoppingStrategy.h"
#include "BasicStoppingStrategy.h"


namespace atp
{
namespace search
{


IteratorManager::IteratorManager(
	const logic::KnowledgeKernelPtr& p_ker) :
	m_ker(p_ker)
{
	// sets up the functor
	reset_stopping_strategy();
}


logic::PfStateSuccIterPtr IteratorManager::begin_iteration_of(
	const logic::ProofStatePtr& p_state) const
{
	ATP_SEARCH_ASSERT((bool)construct_stopping_strategy);

	auto iter = p_state->succ_begin();
	auto strat = construct_stopping_strategy();

	if (m_heuristic != nullptr && strat != nullptr)
	{
		return std::make_shared<StoppingIterator>(
			std::move(iter), std::move(strat),
			m_heuristic);
	}
	else return iter;
}


void IteratorManager::set_heuristic(HeuristicPtr p_heuristic)
{
	m_heuristic = std::move(p_heuristic);
}


void IteratorManager::reset_stopping_strategy()
{
	construct_stopping_strategy = []()
	{
		return nullptr;
	};
}


void IteratorManager::set_fixed_stopping_strategy(
	size_t N)
{
	construct_stopping_strategy = [N]()
	{
		return std::make_unique<FixedStoppingStrategy>(N);
	};
}


void IteratorManager::set_basic_stopping_strategy(
	size_t initial_fill, float lambda, float alpha)
{
	construct_stopping_strategy = [initial_fill, lambda, alpha]()
	{
		return std::make_unique<BasicStoppingStrategy>(initial_fill,
			lambda, alpha);
	};
}


}  // namespace search
}  // namespace atp


