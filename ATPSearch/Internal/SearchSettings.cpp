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


SolverPtr create_solver(logic::KnowledgeKernelPtr p_ker,
	HeuristicPtr p_heuristic,
	pt::ptree ptree)
{
	auto p_iter_mgr = std::make_unique<IteratorManager>(p_ker);

	p_iter_mgr->set_heuristic(p_heuristic);

	if (auto stop_strat_ptree =
		ptree.get_child_optional("stopping-strategy"))
	{
		// cannot have a stopping strategy without a heuristic
		if (p_heuristic == nullptr)
			return nullptr;

		if (!try_load_succ_iter_settings(*p_iter_mgr,
			*stop_strat_ptree))
			return nullptr;
	}

	auto solver_ptree = ptree.get_child("solver");
	
	// if the user has provided a solver tag, then we expect
	// the solver they give to be valid - however if this is
	// null, they made a mistake in their specification of
	// the solver.

	return try_create_solver(
		p_ker, solver_ptree, std::move(p_heuristic),
		std::move(p_iter_mgr));
}


bool load_search_settings(logic::KnowledgeKernelPtr p_ker,
	std::istream& in, SearchSettings* p_out_settings)
{
	ATP_SEARCH_PRECOND(p_out_settings != nullptr);
	ATP_SEARCH_PRECOND(p_ker != nullptr);

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

		HeuristicPtr p_heuristic;
		if (auto heuristic_ptree =
			ptree.get_child_optional("heuristic"))
		{
			p_heuristic = try_create_heuristic(*heuristic_ptree);
		}

		if (!ptree.get_child_optional("solver"))
			return true;  // no solver is not an error

		// it is very important that we copy by value here!
		p_out_settings->create_solver = boost::bind(&create_solver,
			p_ker, p_heuristic, ptree);

		// warning: hacky solution for checking if the solver
		// creation was valid:
		if (p_out_settings->create_solver() == nullptr)
		{
			// error! for some reason we cannot produce solvers.

			// reset this
			p_out_settings->create_solver =
				std::function<SolverPtr()>();

			return false;
		}

		// else we are done
		return true;
	}
	catch(pt::ptree_error&)
	{
		// in case we had already loaded this
		p_out_settings->create_solver = std::function<SolverPtr()>();

		return false;
	}

	return true;
}


}  // namespace search
}  // namespace atp


