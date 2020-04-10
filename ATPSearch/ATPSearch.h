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


/**
\brief Checks the given proof from front to back (the front element
    is expected to be trivially true).

\pre ker.valid(proof)

\returns True iff the proof is correct (if each step follows from the
    last.)

\warning This function is called "simple" for two reasons: firstly,
    if the kernel doesn't have \b all proven theorems available to it
    it may return an incorrect result, because the kernel's `follows`
    function depends on having those theorems loaded (if the kernel
    doesn't know about a theorem, how could it know that line
    followed from the last?) Finally, it may be quite a slow function
    as it tries to check the whole proof at once and doesn't allow
    checking it in parts.
*/
ATP_SEARCH_API bool simple_check_proof(logic::IKnowledgeKernel& ker,
	logic::StatementArrayPtr proof);


}  // namespace search
}  // namespace atp


