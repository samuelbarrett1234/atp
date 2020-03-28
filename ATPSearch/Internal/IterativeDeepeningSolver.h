#pragma once


/*


IterativeDeepeningSolver.h


This is an uninformed search strategy for finding proofs. It remains
relatively low memory by mimicking depth first search, but limits the
depth and iteratively increases it. Being an uninformed strategy, it
is likely not very good, and will likely serve as a minimal viable
product component.

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


class ATP_SEARCH_API IterativeDeepeningSolver : public ISolver
{
private:
	struct StackFrame
	{
		logic::StatementArrayPtr m_stmts;
		size_t m_idx;
	};
public:
	// max_depth : the ultimate depth limit of search
	// starting_depth : the depth limit for the very first search on
	// a new batch of target statements.
	// Preconditions: starting_depth > 1 and
	// starting_depth < max_depth
	IterativeDeepeningSolver(logic::KnowledgeKernelPtr p_kernel,
		size_t max_depth,
		size_t starting_depth = 3);

	virtual void set_targets(logic::StatementArrayPtr p_stmts) override;

	virtual void step(size_t n = 1) override;

	virtual void clear() override;

	virtual inline std::vector<ProofState> get_states() const override
	{
		return m_pf_states;
	}

	virtual inline std::vector<boost::optional<logic::StatementArrayPtr>>
		get_proofs() const override
	{
		return m_proofs;
	}

	virtual bool any_proof_not_done() const override;

	virtual inline bool engaged() const override
	{
		return !m_stacks.empty();
	}

	virtual inline std::vector<float> get_agg_time() const override
	{
		return m_agg_time;
	}

	virtual inline std::vector<size_t> get_max_mem() const override
	{
		// One advantage of this approach is that the max memory
		// usage is easy to track; it is just the current depth,
		// as this is the largest the stack frame is allowed to
		// grow.
		return m_cur_depth_limits;
	}

	virtual inline std::vector<size_t> get_num_expansions() const override
	{
		return m_num_node_exps;
	}

private:
	// functions which form part of the step() function:

	// step an individual proof i for n steps
	void step_proof(size_t i, size_t n);

	// expand the back of the stack by adding successors using the
	// kernel. Precondition: st.size() < m_cur_depth_limits[i]
	void expand_next(size_t i, std::list<StackFrame>& st);

	// when we can't expand any more, we have to delete some end
	// elements off the stack. in particular, we need to restore
	// the invariant that st.back().m_stmts.at(st.back().m_idx)
	// is ready to expand!
	void trim_expansion(size_t i, std::list<StackFrame>& st);

private:
	const size_t m_max_depth;  // ultimate depth limit
	const size_t m_starting_depth;

	logic::KnowledgeKernelPtr m_kernel;

	// a different depth limit for each proof
	// this is iteratively increased as the full tree up to that
	// depth limit is explored.
	// invariant: m_cur_depth_limits[i] <= m_max_depth
	std::vector<size_t> m_cur_depth_limits;

	// we store a stack for each target.
	// invariant: m_stacks[i].size() <= m_cur_depth_limits[i]
	// m_stacks[i].back().m_stmts[m_stacks[i].m_idx] is the next
	// element to be expanded, provided depth limits permit.
	// also invariant: m_stacks[i].size() > 0 iff we are still
	// actively trying to prove it (if empty, we have given up.)
	std::vector<
		std::list<StackFrame>
	> m_stacks;

	// invariant: m_targets->size() == m_stacks.size()
	logic::StatementArrayPtr m_targets;

	// invariant: m_proofs.size() == m_stacks.size()
	// and m_proofs[i].has_value() is true iff m_pf_states[i]
	// != ProofState::UNFINISHED
	// and m_stacks[i].empty() => !m_proofs[i].has_value()
	std::vector<boost::optional<logic::StatementArrayPtr>> m_proofs;

	// invariant: m_proofs.size() == m_stacks.size()
	// and m_stacks[i].empty() => m_pf_states[i] == UNFINISHED
	std::vector<ProofState> m_pf_states;

	// invariant: m_agg_time.size() == m_stacks.size()
	// and m_agg_time[i] >= 0 for all i
	// (this is just tracking the total time each proof has taken
	// so far. 'Agg' for aggregation.)
	std::vector<float> m_agg_time;

	// invariant: m_num_node_exps.size() == m_stacks.size()
	// (this tracks the number of nodes (statements) expanded during
	// the proof process.)
	// note that if the knowledge kernel uses lazy evaluation for
	// succs(), this is also equal to the number of nodes allocated.
	std::vector<size_t> m_num_node_exps;

	// invariant: engaged iff !m_stacks.empty()
};


typedef std::shared_ptr<ISolver> SolverPtr;


}  // namespace search
}  // namespace atp


