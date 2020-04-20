/**

\file

\author Samuel Barrett

*/


#include "SearchSettings.h"
#include <chrono>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../Internal/IterativeDeepeningSolver.h"


namespace pt = boost::property_tree;


namespace atp
{
namespace search
{


/**
\brief Try to create the iteration settings flags
*/
logic::IterSettings try_get_flags(const pt::ptree& ptree);


/**
\brief Try to create an IterativeDeepeningSolver from the given ptree

\returns Nullptr on failure, otherwise returns the solver object

\throws Anything that might be thrown by the ptree.
*/
SolverPtr try_create_IDS(logic::KnowledgeKernelPtr p_ker,
	logic::IterSettings settings, const pt::ptree& ptree);


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

		// defaults to empty string (representing the user not
		// wanting to create a solver) if type is not set
		const std::string solver_type = ptree.get<std::string>(
			"type", "");

		auto iter_settings = try_get_flags(ptree);

		if (solver_type == "IterativeDeepeningSolver")
		{
			p_out_settings->p_solver = try_create_IDS(
				p_ker, iter_settings, ptree);

			if (p_out_settings->p_solver == nullptr)
			{
				// failed to create the solver we wanted
				return false;
			}
		}
		// else if... [other solver types go here]
		else if (solver_type.empty())
		{
			// default solver is empty
			p_out_settings->p_solver.reset();
		}
		else  // if (!solver_type.empty())
		{
			return false;  // bad solver type
		}
	}
	catch(pt::ptree_error&)
	{
		// in case we had already loaded this
		p_out_settings->p_solver.reset();

		return false;
	}

	return true;
}


logic::IterSettings try_get_flags(const pt::ptree& ptree)
{
	logic::IterSettings flags = logic::iter_settings::DEFAULT;

	// each of these flags is optional to provide, and if it is
	// provided, it should either be `true` or `false`.

	if (auto b = ptree.get_optional<bool>("no-repeats"))
	{
		if (b.get())
			flags |= logic::iter_settings::NO_REPEATS;
		else
			flags &= ~logic::iter_settings::NO_REPEATS;
	}

	if (auto b = ptree.get_optional<bool>("randomised"))
	{
		if (b.get())
			flags |= logic::iter_settings::RANDOMISED;
		else
			flags &= ~logic::iter_settings::RANDOMISED;
	}

	return flags;
}


SolverPtr try_create_IDS(logic::KnowledgeKernelPtr p_ker,
	logic::IterSettings iter_settings, const pt::ptree& ptree)
{
	return std::make_shared<IterativeDeepeningSolver>(
		p_ker, ptree.get<size_t>("max-depth", 10),
		ptree.get<size_t>("starting-depth", 3),
		iter_settings);
}


}  // namespace search
}  // namespace atp


