#pragma once


/*


ISolver.h


Solvers are the core interface for the search procedure. They are
given a fixed language and knowledge kernel, and then you can set
target statements for them, which they will attempt to prove by
using the reductions available from the knowledge kernel.
Solvers are vectorised for efficiency; each thread is thus trying
to prove many statements at once.
Furthermore, solvers are designed to work in 'steps', allowing you
to execute arbitrary code in-between.

*/


#include <memory>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/// <summary>
/// Proofs can either be finished or unfinished; and if they are
/// finished, the result can either be that the theorem was true
/// or it was false.
/// </summary>
enum class ProofState
{
	UNFINISHED,
	DONE_TRUE,
	DONE_FALSE
};


/// <summary>
/// A generic interface for proving statements.
/// Solvers are given a fixed language and knowledge kernel during
/// initialisation, which remains fixed throughout the object's
/// lifetime.
/// </summary>
class ATP_SEARCH_API ISolver
{
public:
	virtual ~ISolver() = default;

	// Precondition: p_stmts is valid within the knowledge base and
	// is nonempty (p_stmts->size() > 0)
	// Postcondition: calls clear() then sets these statements
	// to be the statements targeted for proving. engaged() will now
	// return true.
	virtual void set_targets(logic::StatementArrayPtr p_stmts) = 0;

	// Precondition: engaged(), n > 0
	// Postcondition: applies n proof steps to each target statement.
	// This function does nothing if all proofs have already
	// terminated.
	virtual void step(size_t n = 1) = 0;

	// Postcondition: removes statement targets, and performance
	// information, and search states (basically resets object state
	// to what it was just after initialisation.)
	// This function is idempotent. engaged() will now return false.
	virtual void clear() = 0;

	// Precondition: engaged()
	// Postcondition: returns the status of each ongoing proof (the
	// returned array will have the same size as the targets which
	// were set previously).
	virtual std::vector<ProofState> get_states() const = 0;

	// Precondition: engaged()
	// Postcondition: for each target, returns a proof if and only if
	// that proof terminated and the theorem was true. A proof
	// constitutes a sequence of statements which are each deduced
	// from the last using exactly one step from the knowledge kernel.
	// If a proof is present in the optional, then check_proof(proof)
	// will return true.
	virtual std::vector<boost::optional<logic::StatementArrayPtr>>
		get_proofs() const = 0;

	// Precondition: engaged()
	// Postcondition: returns !any(get_states() == UNFINISHED)
	virtual bool any_proof_not_done() const = 0;

	// Postcondition: returns true if and only if the solver
	// currently has target statements set, which it is trying
	// to prove. Note that, even if all proofs have terminated,
	// this function still returns true!
	virtual bool engaged() const = 0;

	// Precondition: engaged()
	// Postcondition: returns the total amount of time spent on
	// proving each target statement (thus far).
	virtual std::vector<float> get_agg_time() const = 0;

	// Precondition: engaged()
	// Postconditions: returns the largest number of nodes represented
	// at any one time in the search trees of each proof.
	virtual std::vector<size_t> get_max_mem() const = 0;

	// Preconditions: engaged()
	// Postconditions: returns the number of node expansions
	// performed in the search tree for each proof.
	virtual std::vector<size_t> get_num_expansions() const = 0;
};


typedef std::shared_ptr<ISolver> SolverPtr;


}  // namespace search
}  // namespace atp


