#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a heuristic which uses the edit distance between the
	forefront(s) of a proof state and its target.

*/


#include <ATPStatsEditDistance.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IHeuristic.h"


namespace atp
{
namespace search
{


/**
\brief This heuristic uses a version of edit distance to estimate how
	far a proof state is away from its target.

\details Since this function is highly specific to the structure of
	the proof state, internally this class has to specialise its
	calculations to each logic type. However each implementation will
	still use the edit distance function in some way.
*/
class ATP_SEARCH_API EditDistanceHeuristic :
	public IHeuristic
{
public:
	/**
	\param p_ctx The model context tells us what symbols are present.

	\param p_ker The knowledge kernel tells us what axioms we can use
		hence a "reference point" for computing distances.

	\param symbol_mismatch_cost The edit distance cost for a mismatch
		of symbols, relative to the cost of a free variable
		substitution which is always fixed at 1.0

	\param symbol_match_benefit The corresponding benefit (decrease
		in edit distance) incurred when two symbols match

	\pre symbol_mismatch_cost > 0 and symbol_match_benefit > 0
	*/
	EditDistanceHeuristic(const logic::ModelContextPtr& p_ctx,
		const logic::KnowledgeKernelPtr& p_ker,
		float symbol_mismatch_cost,
		float symbol_match_benefit);

	float predict(const logic::ProofStatePtr& p_state) override;

private:
	logic::KnowledgeKernelPtr m_ker;
	stats::EditDistancePtr m_ed;
};


}  // namespace search
}  // namespace atp


