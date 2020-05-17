/*
\file

\author Samuel Barrett

\details IMPORTANT NOTE: for some heuristics, it makes sense to use
	one shared heuristic for all solvers. For some heuristics this
	does not make sense, and the requirement to be thread safe might
	sacrifice efficiency. Hence, in this file you must decide whether
	the heuristic should be shared or not: if it is to be shared, you
	should create the heuristic in the function below and make the
	HeuristicCreator be a lambda which just returns the already
	created heuristic (this is okay since it's a shared pointer), and
	otherwise you should construct a new heuristic each time in the
	functor.
*/


#include "SearchSettingsHeuristics.h"
#include "EditDistanceHeuristic.h"
#include "../ATPSearchLog.h"


namespace atp
{
namespace search
{


bool try_create_heuristic(
	const logic::ModelContextPtr& p_ctx,
	HeuristicCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const std::string heuristic_type = ptree.get<std::string>(
		"type");

	if (heuristic_type == "EditDistanceHeuristic")
	{
		// may or may not fail
		return try_create_edit_distance_heuristic(p_ctx, creator,
			ptree);
	}
	else
	{
		ATP_SEARCH_LOG(error) << "Search settings error: bad "
			"heuristic type \"" << heuristic_type << '"';

		return false;  // bad heuristic type
	}
}


bool try_create_edit_distance_heuristic(
	const logic::ModelContextPtr& p_ctx,
	HeuristicCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const float symb_mismatch_cost =
		ptree.get<float>("symbol-mismatch-cost");
	const float symb_match_benefit =
		ptree.get<float>("symbol-match-benefit");

	if (symb_match_benefit <= 0.0f || symb_mismatch_cost <= 0.0f)
	{
		ATP_SEARCH_LOG(error) << "Search settings error: bad edit "
			"distance parameters.";

		return false;
	}

	/*
	Do NOT share this heuristic between solvers, it is not thread-
	safe, so instead create a new one for each solver.
	*/

	creator = [p_ctx, symb_match_benefit, symb_mismatch_cost](
		const logic::KnowledgeKernelPtr& p_ker)
	{
		return std::make_shared<EditDistanceHeuristic>(p_ctx, p_ker,
			symb_mismatch_cost, symb_match_benefit);
	};

	return true;
}


}  // namespace search
}  // namespace atp


