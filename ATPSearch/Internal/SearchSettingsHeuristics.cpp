/*
\file

\author Samuel Barrett
*/


#include "SearchSettingsHeuristics.h"
#include "EditDistanceHeuristic.h"


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
		return false;  // bad heuristic type
	}
}


bool try_create_edit_distance_heuristic(
	const logic::ModelContextPtr& p_ctx,
	HeuristicCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const float p = ptree.get<float>("p");
	const float symb_mismatch_cost =
		ptree.get<float>("symbol-mismatch-cost");

	if (p <= 0.0f || symb_mismatch_cost <= 0.0f)
		return false;  // bad parameters

	creator = [p_ctx, p, symb_mismatch_cost](
		const logic::KnowledgeKernelPtr& p_ker)
	{
		return std::make_shared<EditDistanceHeuristic>(p_ctx, p_ker,
			symb_mismatch_cost, p);
	};

	return true;
}


}  // namespace search
}  // namespace atp


