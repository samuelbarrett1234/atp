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
#include <ATPLogic.h>
#include "Interfaces/ISolver.h"
#include "Interfaces/IStatementHeuristic.h"


/**

\namespace atp::search

\brief The namespace of the ATPSearch library.
*/


namespace atp
{
namespace search
{


/**
\brief The class of different solvers available
*/
enum class SolverType
{
	ITERATIVE_DEEPENING_UNINFORMED
};


/**
\brief Allocate a new solver object of a given specified type

\param ms Some solvers may need specific heuristic functions,
    which can be provided through this argument.

\todo We perhaps need a better way of supplying auxiliary
    parameters to the solvers.
*/
ATP_SEARCH_API SolverPtr create_solver(
    logic::KnowledgeKernelPtr p_ker,
    SolverType st,
	HeuristicCollection ms);


}  // namespace search
}  // namespace atp


