#pragma once

/**
\file

\author Samuel Barrett

\brief Contains a class for managing the handling of successor
	iteration within a solver.
*/


#include <functional>
#include <Interfaces/IProofState.h>
#include <Interfaces/IKnowledgeKernel.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IHeuristic.h"
#include "../Interfaces/IStoppingStrategy.h"


namespace atp
{
namespace search
{


/**
\brief Handles the construction of successor iterators for solvers

\details There are many different settings for iteration (some
	represented by flags in the logic::IterSettings, others
	represented by heuristics and stopping strategies.) This class
	encapsulates the setting up of these iterators from a proof state
	hence reducing the work for any given solver.
*/
class ATP_SEARCH_API IteratorManager
{
public:
	IteratorManager(const logic::KnowledgeKernelPtr& p_ker);

	/**
	\brief Construct a begin iterator of this state's successors.

	\returns A successor iterator with any heuristics / stopping
		strategies applied.
	*/
	logic::PfStateSuccIterPtr begin_iteration_of(
		const logic::ProofStatePtr& p_state) const;

	/**
	\note The heuristic object will remain shared between whoever
		owned this pointer last. However this object will store a
		strong reference, so it may be destroyed elsewhere.
	*/
	void set_heuristic(HeuristicPtr p_heuristic);

	/**
	\brief Set NO stopping strategy
	*/
	void reset_stopping_strategy();

	/**
	\brief Set the stopping strategy to be a FixedStoppingStrategy
		with size N.
	*/
	void set_fixed_stopping_strategy(size_t N);


	/**
	\brief Set the stopping strategy to be a BasicStoppingStrategy
	*/
	void set_basic_stopping_strategy(size_t initial_fill,
		float lambda, float alpha);

private:
	logic::KnowledgeKernelPtr m_ker;
	HeuristicPtr m_heuristic;
	std::function<StoppingStrategyPtr()> construct_stopping_strategy;
};


}  // namespace search
}  // namespace atp


