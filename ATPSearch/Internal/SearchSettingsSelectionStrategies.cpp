/**
\file

\author Samuel Barrett

*/


#include "SearchSettingsSelectionStrategies.h"
#include "FixedSelectionStrategy.h"
#include "EditDistanceSelectionStrategy.h"
#include "SearchSettingsSelectionStrategies.h"


namespace atp
{
namespace search
{


bool try_create_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator)
{
	// compulsory
	const std::string type = ptree.get<std::string>("type");

	if (type == "FixedSelectionStrategy")
	{
		return try_create_fixed_selection_strategy(ptree, creator);
	}
	else if (type == "EditDistanceSelectionStrategy")
	{
		return try_create_edit_dist_selection_strategy(ptree,
			creator);
	}
	else  // not default, but bad type
	{
		return false;
	}
}


bool try_create_fixed_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator)
{
	const size_t n = ptree.get<size_t>("num-helper-thms",
		10);

	creator = [n](const logic::ModelContextPtr& p_ctx, size_t ctx_id)
		-> SelectionStrategyPtr
	{
		// TEMP! Todo: make it easier to get access to the logic language

		auto p_lang = logic::create_language(logic::LangType::EQUATIONAL_LOGIC);

		return std::make_unique<FixedSelectionStrategy>(p_lang, p_ctx,
			ctx_id, n);
	};

	return true;
}


bool try_create_edit_dist_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator)
{
	const size_t num_load = ptree.get<size_t>("num-thms-load",
		20);
	const size_t num_help = ptree.get<size_t>("num-thms-help",
		10);
	const float symb_match_benefit = ptree.get<float>(
		"symbol-match-benefit", 5.0f);
	const float symb_mismatch_cost = ptree.get<float>(
		"symbol-mismatch-cost", 5.0f);

	// check for bad inputs:
	if (num_load < num_help)
	{
		return false;
	}

	creator = [num_load, num_help, symb_match_benefit,
		symb_mismatch_cost](const logic::ModelContextPtr& p_ctx,
			size_t ctx_id)
		-> SelectionStrategyPtr
	{
		// TEMP! Todo: make it easier to get access to the logic language

		auto p_lang = logic::create_language(logic::LangType::EQUATIONAL_LOGIC);

		return std::make_unique<EditDistanceSelectionStrategy>(p_lang, p_ctx,
			ctx_id, num_load, num_help, symb_match_benefit,
			symb_mismatch_cost);
	};

	return true;
}

}  // namespace search
}  // namespace atp


