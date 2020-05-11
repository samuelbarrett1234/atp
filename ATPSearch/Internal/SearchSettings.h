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

	// the number of theorems, loaded from the database, to help
	// prove the statements (these are carried forward to the solver
	// and increase the branching factor).
	size_t num_helper_thms;

	// we will load num_helper_thms * helper_thms_factor proven
	// theorems from the database, and pick the best from them.
	// increasing this will increase load times, but will increase
	// the likelihood of finding a good set of helper theorems.
	size_t helper_thms_factor;

	// this is the cost of a symbol mismatch for edit distance
	// calculations - it is relative to the cost of a free variable
	// substitution, which is fixed at 1.0
	// you will want to experiment with this to see which picks the
	// best helper theorems.
	float ed_symb_mismatch_cost;

	// random number generator seed
	size_t seed;
	
	// a functor for creating solvers using the search settings that
	// have been loaded.
	std::function<SolverPtr(logic::KnowledgeKernelPtr)
		> create_solver;
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
	p_solver will be set to null but this may not be an error. Check
	the return value!

*/
ATP_SEARCH_API bool load_search_settings(
	const logic::ModelContextPtr& p_ctx,
	std::istream& in, SearchSettings* p_out_settings);


}  // namespace search
}  // namespace atp


