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
	const stats::EditDistSubCosts& sub_costs,
	float p);


EditDistanceHeuristic::EditDistanceHeuristic(
	const logic::ModelContextPtr& p_ctx,
	const logic::KnowledgeKernelPtr& p_ker,
	float symbol_mismatch_cost, float p) :
	m_ker(p_ker), m_p(p)
{
	ATP_SEARCH_PRECOND(m_p > 0.0f);
	ATP_SEARCH_PRECOND(symbol_mismatch_cost > 0.0f);

	// get all the symbols in the model context

	m_all_symbols = p_ctx->all_function_symbol_ids();
	auto temp_arr = p_ctx->all_constant_symbol_ids();
	m_all_symbols.insert(m_all_symbols.end(),
		temp_arr.begin(), temp_arr.end());

	// construct substitution cost mapping
	for (size_t id1 : m_all_symbols)
	{
		for (size_t id2 : m_all_symbols)
		{
			if (p_ctx->symbol_arity(id1)
				>= p_ctx->symbol_arity(id2))
			{
				m_sub_costs[std::make_pair(id1, id2)]
					= ((id1 == id2) ? 0.0f : symbol_mismatch_cost);
			}
		}
	}
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
			m_sub_costs, m_p);
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
	const stats::EditDistSubCosts& sub_costs,
	float p)
{
	// this includes axioms, but also loaded helper theorems
	const auto& axioms = ker.get_active_rules();

	auto forefront = logic::from_statement(pf_state.forefront());

	const auto dists = stats::pairwise_edit_distance(*forefront, axioms,
		sub_costs);

	ATP_SEARCH_ASSERT(dists.size() == 1);

	float utility = 0.0f;
	for (float x : dists.front())
	{
		ATP_SEARCH_ASSERT(x >= 0.0f);

		if (x == 0.0f)
			utility += 1.0f;
		else
			utility += 1.0f / (std::powf(x, p) + 1.0f);
	}

	return utility;
}


}  // namespace search
}  // namespace atp


