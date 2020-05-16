#pragma once


/**

\file

\author Samuel Barrett

\brief Contains a basic uninformed search strategy, IDS.

\details This is an uninformed search strategy for finding proofs. It
    remains relatively low memory by mimicking depth first search,
	but limits the depth and iteratively increases it. Being an
	uninformed strategy, it is likely not very good, and will likely
	serve as a minimal viable product component.

*/


#include <memory>
#include <list>
#include <vector>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/ISolver.h"


namespace atp
{
namespace search
{


class IteratorManager;  // forward declaration


class ATP_SEARCH_API IterativeDeepeningSolver : public ISolver
{
private:
	/**
	\brief A single element in the DFS stack
	*/
	struct StackFrame
	{
		// `iter` starts off as node->succ_begin()

		logic::ProofStatePtr node;
		logic::PfStateSuccIterPtr iter;

		// equal to the number of times the iterator has been
		// advanced
		size_t iter_off;
	};

public:
	/**

	\param max_depth The ultimate depth limit of search (if a proof
	    is not found at this depth, the search terminates.)

	\param starting_depth The depth limit of the very first search
	    iteration.

	\pre 1 < starting_depth < max_depth

	*/
	IterativeDeepeningSolver(logic::KnowledgeKernelPtr p_kernel,
		size_t max_depth,
		size_t starting_depth = 3,
		size_t width_limit = 50,
		size_t width_limit_start = 2,
		logic::IterSettings iter_settings =
		logic::iter_settings::DEFAULT,
		std::unique_ptr<IteratorManager> p_iter_mgr = 
		std::unique_ptr<IteratorManager>());

	void set_targets(logic::StatementArrayPtr p_stmts) override;

	void step(size_t n = 1) override;

	void clear() override;

	inline std::vector<logic::ProofCompletionState> get_states() const override
	{
		return m_pf_states;
	}

	inline std::vector<logic::ProofStatePtr>
		get_proofs() const override
	{
		return m_proofs;
	}

	bool any_proof_not_done() const override;

	inline bool engaged() const override
	{
		return !m_stacks.empty();
	}

	inline std::vector<float> get_agg_time() const override
	{
		return m_agg_time;
	}

	inline std::vector<size_t> get_max_mem() const override
	{
		return m_max_mem;
	}

	inline std::vector<size_t> get_num_expansions() const override
	{
		return m_num_node_exps;
	}

private: 
	/**
	\brief Step an individual proof i for n steps
	*/
	void step_proof(size_t i, size_t n);


	/**
	\brief Expand the next available node (i.e. examine whatever
		the back iterator is pointing to)

	\pre m_stacks[i].back().iter->valid()

	\post m_stacks[i].back().iter->valid()
	*/
	void expand_next(size_t i);

	/**
	\brief Handle the completion of current search of target statement i

	\param i The index of the target statement (whose stack is
	    represented by m_stacks[i]).

	\pre m_stacks[i].back().iter->get()->completion_state() == PROVEN
	*/
	void finish(size_t i);

	/**
	\brief Count the number of nodes stored in the stack of target i
	*/
	size_t count_mem(size_t i) const;

	/**
	\brief Initialise m_stacks[i] with a new proof state targetting
		m_targets->at(i).

	\pre m_stacks[i].empty()

	\post !m_stacks[i].empty() or m_pf_states[i] == NO_PROOF

	\details This has been factored out into a function because there
		are two places where we initialise the stack, and I wanted to
		avoid repetition.
	*/
	void init_stack(size_t i);

private:
	const logic::IterSettings m_iter_settings;
	const size_t m_max_depth;  // ultimate depth limit
	const size_t m_starting_depth;
	const size_t m_width_limit;
	const size_t m_width_limit_start_depth;

	logic::KnowledgeKernelPtr m_kernel;
	std::unique_ptr<IteratorManager> m_iter_mgr;

	/**
	\brief Depth limit for each target statement

	\details This is iteratively increased as the search progresses
	    until the ultimate depth limit has been reached.

	\invariant m_cur_depth_limits[i] <= m_max_depth

	*/
	std::vector<size_t> m_cur_depth_limits;

	/**

	\brief We store a stack for each target statement.

	\invariant m_stacks[i].size() <= m_cur_depth_limits[i]

	\invariant m_stacks[i].size() > 0 iff we are still
	    actively trying to prove it (if empty, we have given up.)

	\invariant engaged iff !m_stacks.empty()

	\details m_stacks[i].back().m_stmts[m_stacks[i].m_idx] is the
	    next element to be expanded, provided depth limits permit.

	*/
	std::vector<
		std::list<StackFrame>
	> m_stacks;


	/**
	\invariant m_targets->size() == m_stacks.size()
	*/
	logic::StatementArrayPtr m_targets;

	
	/**
	\invariant m_proofs.size() == m_stacks.size()
	    and m_proofs[i] != nullptr iff m_pf_states[i]
		!= logic::ProofCompletionState::UNFINISHED
	*/
	std::vector<logic::ProofStatePtr> m_proofs;


	/**
	\invariant m_proofs.size() == m_stacks.size()
	    and m_stacks[i].empty() => m_pf_states[i] == UNFINISHED
	*/
	std::vector<logic::ProofCompletionState> m_pf_states;


	/**
	\invariant m_agg_time.size() == m_stacks.size()
	    and m_agg_time[i] >= 0 for all i

	\brief Tracks the total time each proof has taken so far, with
	    "agg" meaning "aggregated".
	*/
	std::vector<float> m_agg_time;

	
	/**
	\invariant m_num_node_exps.size() == m_stacks.size()

	\brief Tracks the number of nodes (statements) expanded during
	    the proof process.

	\details If the knowledge kernel uses lazy evaluation for `succs`
	    then this is also equal to the number of nodes allocated.
	*/
	std::vector<size_t> m_num_node_exps;


	/**
	\invariant m_max_mem.size() == m_stacks.size()

	\brief Tracks the maximum number of node stored at once by each
	    proof.

	\details If the knowledge kernel uses lazy evaluation for `succs`
		then this a loose upper bound, and the true value is just the
		depth (the stack size).
	*/
	std::vector<size_t> m_max_mem;
};


typedef std::shared_ptr<ISolver> SolverPtr;


}  // namespace search
}  // namespace atp


