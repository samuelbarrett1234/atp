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

	\param p A tweakable parameter for computing the heuristic. This
		is the "power to which the inverse of the edit distance is
		raised", which is then summed across all axioms. The higher
		this is, the closer the heuristic will be to computing the
		closest axiom, whereas if this is lower it's more like an
		average.

	\pre p > 0 and symbol_mismatch_cost > 0
	*/
	EditDistanceHeuristic(const logic::ModelContextPtr& p_ctx,
		const logic::KnowledgeKernelPtr& p_ker,
		float symbol_mismatch_cost,
		float p);

	float predict(const logic::ProofStatePtr& p_state) override;

private:
	logic::KnowledgeKernelPtr m_ker;
	std::vector<size_t> m_all_symbols;
	stats::EditDistancePtr m_ed;
	const float m_p;
};


}  // namespace search
}  // namespace atp


