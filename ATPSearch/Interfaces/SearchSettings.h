#pragma once


/**

\file

\author Samuel Barrett

\brief Contains code for loading user search settings

*/


#include <istream>
#include "../ATPSearchAPI.h"
#include <ATPLogic.h>
#include "ISolver.h"


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
	SolverPtr p_solver;

	// name of configuration and a brief description
	std::string name, desc;

	size_t step_size;  // the number of steps to perform in one go

	// the maximum number of calls to step() before giving up
	size_t max_steps;
};


/**
\brief Try to load a solver from a configuration file.

\returns Nullptr if failed, otherwise returns the constructed solver.

\param in The input stream from which to read the JSON solver
	configuration format.

\param p_out_settings The settings struct which we will write our
	search settings to.

\pre p_out_settings != nullptr && p_ker != nullptr

\returns True iff success.

\warning If the user doesn't provide certain info, default values may
	be set instead - for example, if the file provides no solver,
	p_solver will be set to null but this may not be an error. Check
	the return value!

*/
ATP_SEARCH_API bool load_search_settings(
	logic::KnowledgeKernelPtr p_ker,
	std::istream& in, SearchSettings* p_out_settings);


}  // namespace search
}  // namespace atp

