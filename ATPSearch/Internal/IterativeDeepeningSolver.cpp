/**

\file

\author Samuel Barrett

*/


#include "IterativeDeepeningSolver.h"
#include <boost/timer/timer.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include "IteratorManager.h"


namespace phxarg = boost::phoenix::arg_names;


namespace atp
{
namespace search
{


IterativeDeepeningSolver::IterativeDeepeningSolver(
	logic::KnowledgeKernelPtr p_kernel,
	size_t max_depth,
	size_t starting_depth,
	logic::IterSettings iter_settings,
	std::unique_ptr<IteratorManager> p_iter_mgr) :
	m_kernel(p_kernel),
	m_max_depth(max_depth),
	m_starting_depth(starting_depth),
	m_iter_settings(iter_settings),
	m_iter_mgr(std::move(p_iter_mgr))
{
	ATP_SEARCH_PRECOND(starting_depth > 1);
	ATP_SEARCH_PRECOND(max_depth > starting_depth);
	ATP_SEARCH_PRECOND(m_kernel != nullptr);
	ATP_SEARCH_PRECOND(m_iter_mgr != nullptr);
	ATP_SEARCH_PRECOND(m_kernel->iter_settings_supported(
		m_iter_settings));
}


void IterativeDeepeningSolver::set_targets(
	logic::StatementArrayPtr p_stmts)
{
	ATP_SEARCH_PRECOND(p_stmts != nullptr);
	ATP_SEARCH_PRECOND(p_stmts->size() > 0);

	m_targets = p_stmts;
	m_stacks.resize(m_targets->size());
	m_cur_depth_limits.resize(m_stacks.size(), m_starting_depth);
	m_agg_time.resize(m_stacks.size(), 0.0f);
	m_proofs.resize(m_stacks.size());
	m_pf_states.resize(m_stacks.size(),
		logic::ProofCompletionState::UNFINISHED);

	// 1 for the root node:
	m_max_mem.resize(m_stacks.size(), 1);
	m_num_node_exps.resize(m_stacks.size(), 1);

	// construct root nodes for each stack
	for (size_t i = 0; i < m_stacks.size(); i++)
	{
		init_stack(i);
	}
}


void IterativeDeepeningSolver::step(size_t n)
{
	ATP_SEARCH_PRECOND(engaged());
	ATP_SEARCH_PRECOND(n > 0);

	boost::timer::cpu_timer timer;
	timer.stop();  // timer starts by default when constructed

	for (size_t i = 0; i < m_stacks.size(); i++)
	{
		if (m_pf_states[i] ==
			logic::ProofCompletionState::UNFINISHED)
		{
			// start the timer individually for each proof
			// .start() resets timer to zero whereas .resume()
			// starts from the time we left off.
			timer.start();

			step_proof(i, n);

			// add the total time to our tracker:
			timer.stop();
			auto time = timer.elapsed();
			m_agg_time[i] += static_cast<float>(time.user
				+ time.system) * 1.0e-9f;  // convert to seconds
		}
#ifdef ATP_SEARCH_DEFENSIVE
		else  // check the state is all consistent
		{
			ATP_SEARCH_ASSERT(m_stacks[i].empty());
			ATP_SEARCH_ASSERT(m_proofs[i] != nullptr);
		}
#endif
	}
}


void IterativeDeepeningSolver::clear()
{
	m_targets.reset();
	m_stacks.clear();
	m_proofs.clear();
	m_cur_depth_limits.clear();
	m_pf_states.clear();
	m_agg_time.clear();
	m_num_node_exps.clear();
	m_max_mem.clear();
}


bool IterativeDeepeningSolver::any_proof_not_done() const
{
	ATP_SEARCH_PRECOND(engaged());
	return std::any_of(m_pf_states.begin(),
		m_pf_states.end(),
		phxarg::arg1 == logic::ProofCompletionState::UNFINISHED);
}


void IterativeDeepeningSolver::step_proof(size_t i, size_t n)
{
	for (size_t t = 0;
		t < n &&
		m_pf_states[i] == logic::ProofCompletionState::UNFINISHED;
		++t)
	{
		expand_next(i);

		ATP_SEARCH_ASSERT(m_stacks[i].size() <=
			m_cur_depth_limits[i]);

		// update max mem count for this proof
		const auto mem_count = count_mem(i);
		if (mem_count > m_max_mem[i])
			m_max_mem[i] = mem_count;
	}
}


void IterativeDeepeningSolver::expand_next(size_t i)
{
	ATP_SEARCH_PRECOND(i < m_stacks.size());
	auto& st = m_stacks[i];

	ATP_SEARCH_ASSERT(st.back().iter != nullptr);
	ATP_SEARCH_PRECOND(st.back().iter->valid());

	auto expand_candidate = st.back().iter->get();

	// we have expanded another node:
	m_num_node_exps[i]++;

	switch (expand_candidate->completion_state())
	{
	case logic::ProofCompletionState::PROVEN:
		finish(i);
		return;  // EXIT, WE ARE DONE!!!

	case logic::ProofCompletionState::NO_PROOF:
		// try next candidate, this one won't do
		st.back().iter->advance();
		break;

	case logic::ProofCompletionState::UNFINISHED:
		if (st.size() < m_cur_depth_limits[i])
		{
			auto iter = 
				m_iter_mgr->begin_iteration_of(expand_candidate);
			
			// if the begin iterator is invalid the proof state
			// should've returned completion state as `NO_PROOF`
			ATP_SEARCH_PRECOND(iter->valid());

			// explore this candidate if depth limits permit
			st.push_back({
				expand_candidate,
				iter
				});
		}
		// else do nothing because we aren't allowed to go any deeper
		else st.back().iter->advance();

		break;
	}

	// if we have exhausted the back frame
	if (!st.back().iter->valid())
	{
		// restore the stack back to a state where it is either empty
		// or its back iterator is valid
		do
		{
			st.pop_back();

			// advance if nonempty
			if (!st.empty())
			{
				ATP_SEARCH_ASSERT(st.back().iter != nullptr &&
					st.back().iter->valid());
				st.back().iter->advance();
			}

		} while (!st.empty() && !st.back().iter->valid());

		// now either the stack is empty or the iterator is valid
		// and pointing to the next node we should expand
		if (st.empty())
		{
			if (m_cur_depth_limits[i] < m_max_depth)
			{
				// we have exhausted the search up to the current
				// depth limit, and found nothing, however we haven't
				// got to the ultimate depth limit yet, so try again
				// with more depth:

				++m_cur_depth_limits[i];

				// setup the stack again (because it has been
				// emptied)
				init_stack(i);
			}
			else
			{
				// we have reached the ultimate depth limit with no
				// proof, so just bail:
				m_pf_states[i] = logic::ProofCompletionState::NO_PROOF;
			}
		}
		// else we are all good and can handle it on the next step
	}
}


void IterativeDeepeningSolver::finish(size_t i)
{
	ATP_SEARCH_PRECOND(i < m_stacks.size());
	auto& st = m_stacks[i];
	ATP_SEARCH_ASSERT(st.back().iter->valid());
	ATP_SEARCH_PRECOND(st.back().iter->get()->completion_state()
		== logic::ProofCompletionState::PROVEN);

	m_proofs[i] = st.back().iter->get();
	m_pf_states[i] = logic::ProofCompletionState::PROVEN;
	st.clear();
}


size_t IterativeDeepeningSolver::count_mem(size_t i) const
{
	size_t count = 0;

	for (const auto& st_frame : m_stacks[i])
	{
		count += st_frame.iter->size();
	}

	return count;
}


void IterativeDeepeningSolver::init_stack(size_t i)
{
	ATP_SEARCH_PRECOND(i < m_stacks.size());
	ATP_SEARCH_PRECOND(m_stacks[i].empty());

	m_stacks[i].push_back(
		StackFrame{
			m_kernel->begin_proof_of(m_targets->at(i),
			m_iter_settings),
			logic::PfStateSuccIterPtr()
		}
	);

	if (m_stacks[i].back().node->completion_state()
		!= logic::ProofCompletionState::UNFINISHED)
	{
		// the proof is just this one step
		m_proofs[i] = m_stacks[i].back().node;
		m_pf_states[i] = m_stacks[i].back().node->completion_state();
		m_stacks[i].clear();
	}
	else
	{
		// start off the iterator
		m_stacks[i].back().iter =
			m_iter_mgr->begin_iteration_of(m_stacks[i].back().node);
		ATP_SEARCH_ASSERT(m_stacks[i].back().iter->valid());
	}
}


}  // namespace search
}  // namespace atp


