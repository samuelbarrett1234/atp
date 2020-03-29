#pragma once


/*

ATPSearch.h

Main header file for this library.

*/


#include "ATPSearchAPI.h"
#include "Interfaces/ISolver.h"
#include "Interfaces/IStatementHeuristic.h"


namespace atp
{
namespace search
{


// The class of solvers available
enum class SolverType
{
	ITERATIVE_DEEPENING_UNINFORMED
};


ATP_SEARCH_API SolverPtr create_solver(SolverType st,
	HeuristicCollection ms);


// Precondition: ker.valid(proof)
// Postcondition: returns true iff the proof is correct.
// Checks the given proof by ensuring that each statement is reached
// from the last statement in exactly one step according to the
// knowledge kernel.
// WARNING: we call this function "simple" because of two reasons: if
// the kernel doesn't have all proven theorems available to it, it
// may return an incorrect result (which would happen if the proof
// used a theorem which the kernel didn't know about), and it also
// tries to do the proof check all at once, which may be very slow.
ATP_SEARCH_API bool simple_check_proof(logic::IKnowledgeKernel& ker,
	logic::StatementArrayPtr proof);


}  // namespace search
}  // namespace atp


