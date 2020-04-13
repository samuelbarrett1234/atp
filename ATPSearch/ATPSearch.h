#pragma once


/**

\file

\author Samuel Barrett

\brief Main include file for the ATPSearch library

\details It should be possible to use all of this search library by
    just including this file, unless you want specific access to
	certain object interfaces.

*/


#include "ATPSearchAPI.h"
#include "Interfaces/ISolver.h"
#include "Interfaces/IStatementHeuristic.h"
#include "Interfaces/SearchSettings.h"


/**

\namespace atp::search

\brief The namespace of the ATPSearch library.
*/


namespace atp
{
namespace search
{


/**
\brief Create this library's designated "default solver"

\details This is for library users that don't care about the solver
	chosen, or that have no information about what solver they want.
	The returned solver is supposed to be generic and just work "out
	of the box."

\returns A new solver object.

*/
ATP_SEARCH_API SolverPtr create_default_solver(
	logic::KnowledgeKernelPtr p_ker);


}  // namespace search
}  // namespace atp


