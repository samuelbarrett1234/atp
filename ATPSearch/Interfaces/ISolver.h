#pragma once


/**

\file

\author Samuel Barrett

\brief Interface for proof solving strategies

\details Solvers are the core interface for the search procedure.
    They are given a fixed language and knowledge kernel, and then
	you can set target statements for them, which they will attempt
	to prove by using the reductions available from the knowledge
	kernel. Solvers are vectorised for efficiency; each thread is
	thus trying to prove many statements at once. Furthermore, solvers
	are designed to work in 'steps', allowing you to execute arbitrary
	code in-between.

*/


#include <memory>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/**
\details Proofs can either be finished or unfinished; and if they are
    finished, the result can either be that the theorem was true or
	false.
*/
enum class ProofState
{
	UNFINISHED,
	DONE_TRUE,
	DONE_FALSE
};


/**
\interface ISolver

\brief A generic interface for proving statements.

\details Solvers are given a fixed language and knowledge kernel
    during initialisation, which remains fixed throughout the
	object's lifetime.
*/
class ATP_SEARCH_API ISolver
{
public:
	virtual ~ISolver() = default;


	/**
	\pre p_stmts is valid within the knowledge base and is nonempty

	\post calls clear() then sets these statements as the new ones to
	    be targeted for proving.

	\post engaged() will subsequently return true.
	*/
	virtual void set_targets(logic::StatementArrayPtr p_stmts) = 0;


	/**
	\pre engaged() and n > 0

	\post applies n proof steps to each target statement. This
	    function does nothing if all proofs have already terminated.
	*/
	virtual void step(size_t n = 1) = 0;
	

	/**
	\post Removes statement targets and performance information and
	    intermediate search states (basically resets to the state
		immediately after initialisation.)

	\post engaged() will subsequently return false.

	\remark This function is idempotent - it does nothing when not
	    already engaged.
	*/
	virtual void clear() = 0;

	
	/**
	\pre engaged()

	\post returns the status of each proof (returning an array of
	    size equal to the number of set target statements.)
	*/
	virtual std::vector<ProofState> get_states() const = 0;


	/**
	\pre engaged()

	\post For each target, returns a proof iff that proof actually
	    terminated and the theorem was true when it terminated. A
		proof constitutes a sequence of statements which are each
		deduced from the last using exactly one step from the
		knowledge kernel.
	*/
	virtual std::vector<boost::optional<logic::StatementArrayPtr>>
		get_proofs() const = 0;

	/**
	\pre engaged()

	\returns `!any(get_states() == UNFINISHED)`
	*/
	virtual bool any_proof_not_done() const = 0;


	/**
	\returns true iff the solver currently has target statements
	    set (regardless of whether or not those proofs have
		terminated.)
	*/
	virtual bool engaged() const = 0;

	
	/**
	\pre engaged()

	\returns The total amount of time spent proving each target
	    statement thus far.
	*/
	virtual std::vector<float> get_agg_time() const = 0;


	/**
	\pre engaged()

	\returns The largest number of nodes stored at any one time in
	    the search trees for each proof.
	*/
	virtual std::vector<size_t> get_max_mem() const = 0;


	/**
	\pre engaged()
	
	\returns The number of node expansions performed in the searching
	    process for each proof.
	*/
	virtual std::vector<size_t> get_num_expansions() const = 0;
};


typedef std::shared_ptr<ISolver> SolverPtr;


}  // namespace search
}  // namespace atp


