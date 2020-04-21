#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an iterator for wrapping around another iterator and
	enumerating the successors of the child iterator using an optimal
	stopping strategy.

*/


#include <queue>
#include <Interfaces/IProofState.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IStoppingStrategy.h"
#include "../Interfaces/IHeuristic.h"


namespace atp
{
namespace search
{


/**
\brief Wraps around an iterator by extracting several successors at
    a time from it, and returning the "best" one seen so far.

\see atp::logic::IPfStateSuccIter

\details Computation of successors can be expensive, but the order in
    which they are evaluated is not necessarily optimal. This
    iterator tries to return successors in (approximately) non-
    increasing order of "benefit". It does so by extracting several
    and returning the best. However, extraction is expensive, so
    there is a tradeoff to be struck between extracting lots of
    successors vs returning the best seen so far. This is an optimal-
    stopping problem.
*/
class ATP_SEARCH_API OptimalStoppingIterator :
	public logic::IPfStateSuccIter
{
private:
    // comparator object for the priority queue below
    struct ATP_SEARCH_API PQComparator
    {
        inline bool operator()(const std::pair<float,
            logic::ProofStatePtr>& a, const std::pair<float,
            logic::ProofStatePtr>& b) const
        {
            return a.first > b.first;
        }
    };

public:
    OptimalStoppingIterator(logic::PfStateSuccIterPtr child,
        StoppingStrategyPtr stopping_strategy,
        HeuristicPtr benefit_heuristic);

    bool valid() const override;
    logic::ProofStatePtr get() const override;
    void advance() override;
    size_t size() const override;

private:
    /**
    \brief Use the child iterator to extract some elements until the
        stopping strategy stops us or we run out of elements.

    \post m_stopping_strategy->is_stopped() || !m_child->valid()
    */
    void forward();

private:
    logic::PfStateSuccIterPtr m_child;
    StoppingStrategyPtr m_stopping_strategy;
    HeuristicPtr m_benefit_heuristic;

    // stores the states and benefits (the largest benefit being most
    // desirable).
    std::priority_queue<std::pair<float, logic::ProofStatePtr>,
        PQComparator> m_states;
};


}  // namespace search
}  // namespace atp


