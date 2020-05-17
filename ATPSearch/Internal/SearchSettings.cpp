/**

\file

\author Samuel Barrett

*/


#include "SearchSettings.h"
#include <chrono>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "SearchSettingsHeuristics.h"
#include "SearchSettingsSuccIters.h"
#include "SearchSettingsSolvers.h"
#include "IteratorManager.h"


namespace pt = boost::property_tree;


namespace atp
{
namespace search
{


bool create_solver_creator(
	std::function<SolverPtr(
		logic::KnowledgeKernelPtr)>& creator,
	HeuristicCreator heuristic_creator,
	pt::ptree ptree)
{
	SuccIterCreator succ_iter_creator;
	SolverCreator solver_creator;

	if (auto stop_strat_ptree =
		ptree.get_child_optional("stopping-strategy"))
	{
		// cannot have a stopping strategy without a heuristic
		if (!(bool)heuristic_creator)
			return false;

		if (!try_load_succ_iter_settings(succ_iter_creator,
			*stop_strat_ptree))
			return false;

		ATP_SEARCH_ASSERT((bool)succ_iter_creator);
	}

	auto solver_ptree = ptree.get_child("solver");
	
	// if the user has provided a solver tag, then we expect
	// the solver they give to be valid - however if this is
	// null, they made a mistake in their specification of
	// the solver.

	if (!try_create_solver(solver_ptree, solver_creator))
		return false;

	ATP_SEARCH_ASSERT((bool)solver_creator);

	// use move capture:
	creator = [succ_iter_creator{ std::move(succ_iter_creator) },
		solver_creator{ std::move(solver_creator) },
		heuristic_creator{ std::move(heuristic_creator) }](
			logic::KnowledgeKernelPtr p_ker)
	{
		ATP_SEARCH_PRECOND(p_ker != nullptr);

		// not optional
		ATP_SEARCH_ASSERT((bool)solver_creator);

		auto p_iter_mgr = std::make_unique<IteratorManager>(p_ker);

		// this is optional
		if ((bool)heuristic_creator)
			p_iter_mgr->set_heuristic(heuristic_creator(p_ker));

		// this is optional
		if ((bool)succ_iter_creator)
			succ_iter_creator(*p_iter_mgr);

		// not optional
		return solver_creator(p_ker, std::move(p_iter_mgr));
	};

	return true;
}


bool load_search_settings(
	const logic::ModelContextPtr& p_ctx,
	std::istream& in,
	SearchSettings* p_out_settings)
{
	ATP_SEARCH_PRECOND(p_out_settings != nullptr);

	pt::ptree ptree;

	try
	{
		pt::read_json(in, ptree);

		p_out_settings->name = ptree.get<std::string>(
			"name", "Unnamed Search Settings");
		p_out_settings->desc = ptree.get<std::string>(
			"desc", "No description.");

		p_out_settings->max_steps = ptree.get<size_t>(
			"max-steps", 10);
		p_out_settings->step_size = ptree.get<size_t>(
			"step-size", 100);

		// check for bad values:
		if (p_out_settings->max_steps == 0 ||
			p_out_settings->step_size == 0)
			return false;

		// the default seed is based on the current time
		if (ptree.get<std::string>("seed", "time") == "time")
		{
			// get a number based on the current time
			p_out_settings->seed = std::chrono::high_resolution_clock
				::now().time_since_epoch().count();
		}
		else
		{
			p_out_settings->seed = ptree.get<size_t>(
				"seed");
		}

		// no selection strategy is not an error
		if (auto selection_strat_ptree =
			ptree.get_child_optional("selection-strategy"))
		{
			if (!try_create_selection_strategy(*selection_strat_ptree,
				p_out_settings->create_selection_strategy))
				return false;
		}

		HeuristicCreator heuristic_creator;
		if (auto heuristic_ptree =
			ptree.get_child_optional("heuristic"))
		{
			if (!try_create_heuristic(
				p_ctx, heuristic_creator, *heuristic_ptree))
				return false;

			ATP_SEARCH_ASSERT((bool)heuristic_creator);
		}
		
		// no solver is not an error
		if (ptree.get_child_optional("solver"))
		{
			if (!create_solver_creator(p_out_settings->create_solver,
				heuristic_creator, ptree))
				return false;
		}

		// else we are done
		return true;
	}
	catch(pt::ptree_error&)
	{
		// in case we had already loaded these
		p_out_settings->create_solver = decltype(
			p_out_settings->create_solver)();
		p_out_settings->create_selection_strategy = decltype(
			p_out_settings->create_selection_strategy)();

		return false;
	}

	return true;
}


}  // namespace search
}  // namespace atp


