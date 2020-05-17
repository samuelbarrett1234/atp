/**
\file

\author Samuel Barrett

*/


#include "EditDistanceHeuristic.h"
#include <Internal/Equational/ProofState.h>


namespace atp
{
namespace search
{


/**
\brief A specialisation of edit distance computation to equational
	logic.
*/
float eqlogic_edit_distance(
	const logic::equational::ProofState& pf_state,
	const logic::equational::KnowledgeKernel& ker,
	const stats::EditDistancePtr& p_ed,
	float p);


EditDistanceHeuristic::EditDistanceHeuristic(
	const logic::ModelContextPtr& p_ctx,
	const logic::KnowledgeKernelPtr& p_ker,
	float symbol_mismatch_cost, float p) :
	m_ker(p_ker), m_p(p)
{
	ATP_SEARCH_PRECOND(m_p > 0.0f);
	ATP_SEARCH_PRECOND(symbol_mismatch_cost > 0.0f);

	// TEMP (todo: don't specialise to equational logic)
	m_ed = stats::create_edit_dist(
		logic::LangType::EQUATIONAL_LOGIC,
		0.1f * symbol_mismatch_cost, symbol_mismatch_cost);
}


float EditDistanceHeuristic::predict(
	const logic::ProofStatePtr& _p_state)
{
	// specialise the prediction to the logic type
	if (auto p_state =
		dynamic_cast<const logic::equational::ProofState*>(
			_p_state.get()))
	{
		return eqlogic_edit_distance(*p_state,
			dynamic_cast<const logic::equational::KnowledgeKernel&>(*m_ker),
			m_ed, m_p);
	}
	else
	{
		ATP_SEARCH_ASSERT(false && "bad logic type!");
		throw std::exception();
	}
}


float eqlogic_edit_distance(
	const logic::equational::ProofState& pf_state,
	const logic::equational::KnowledgeKernel& ker,
	const stats::EditDistancePtr& p_ed,
	float p)
{
	// this includes axioms, but also loaded helper theorems
	const auto& axioms = ker.get_active_rules();

	auto forefront = logic::from_statement(pf_state.forefront());

	const auto dists = p_ed->edit_distance(*forefront, axioms);

	ATP_SEARCH_ASSERT(dists.size() == 1);

	float utility = 0.0f;
	for (float x : dists.front())
	{
		// higher distance means worse, so use negative
		// (note that x may be negative, as edit distance isn't
		// a distance metric, rather just a distance-inspired
		// heuristic)
		utility += -x;
	}

	// divide by size just to normalise things a bit
	return utility / (float)dists.front().size();
}


}  // namespace search
}  // namespace atp


