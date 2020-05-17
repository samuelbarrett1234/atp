#pragma once


/**

\file

\author Samuel Barrett

\brief Contains code for loading user search settings

*/


#include <istream>
#include <functional>
#include "../ATPSearchAPI.h"
#include <ATPLogic.h>
#include "../Interfaces/ISolver.h"
#include "../Interfaces/IHeuristic.h"
#include "../Interfaces/IStoppingStrategy.h"
#include "SearchSettingsSelectionStrategies.h"


namespace atp
{
namespace search
{


/**
\brief Bundles together user settings for performing searches,
	including the constructed solver object, and other settings
	too.
*/
struct ATP_SEARCH_API SearchSettings
{
	// name of configuration and a brief description
	std::string name, desc;

	size_t step_size;  // the number of steps to perform in one go

	// the maximum number of calls to step() before giving up
	size_t max_steps;

	// random number generator seed
	size_t seed;
	
	// a functor for creating solvers using the search settings that
	// have been loaded.
	std::function<SolverPtr(logic::KnowledgeKernelPtr)
		> create_solver;

	// functor for creating selection strategies
	SelectionStrategyCreator create_selection_strategy;
};


/**
\brief Try to load a solver from a configuration file.

\returns Nullptr if failed, otherwise returns the constructed solver.

\param p_ctx The model context

\param in The input stream from which to read the JSON solver
	configuration format.

\param p_out_settings The settings struct which we will write our
	search settings to.

\pre p_out_settings != nullptr

\returns True iff success.

\warning If the user doesn't provide certain info, default values may
	be set instead - for example, if the file provides no solver,
	create_solver will be left empty but this may not be an error. Check
	the return value!

*/
ATP_SEARCH_API bool load_search_settings(
	const logic::ModelContextPtr& p_ctx,
	std::istream& in, SearchSettings* p_out_settings);


}  // namespace search
}  // namespace atp


