/**

\file

\author Samuel Barrett

*/


#include "IterativeDeepeningSolver.h"
#include <boost/timer/timer.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>


namespace phxarg = boost::phoenix::arg_names;


namespace atp
{
namespace search
{


IterativeDeepeningSolver::IterativeDeepeningSolver(
	logic::KnowledgeKernelPtr p_kernel,
	size_t max_depth,
	size_t starting_depth) :
	m_kernel(p_kernel),
	m_max_depth(max_depth),
	m_starting_depth(starting_depth)
{
	ATP_SEARCH_PRECOND(starting_depth > 1);
	ATP_SEARCH_PRECOND(max_depth > starting_depth);
	ATP_SEARCH_PRECOND(m_kernel != nullptr);
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
	m_pf_states.resize(m_stacks.size(), ProofState::UNFINISHED);

	// 1 for the root node:
	m_max_mem.resize(m_stacks.size(), 1);
	m_num_node_exps.resize(m_stacks.size(), 1);

	// construct root nodes for each stack
	for (size_t i = 0; i < m_stacks.size(); i++)
	{
		m_stacks[i].push_back(
			StackFrame{
				m_targets->slice(i, i + 1),
				0
			}
		);

		// handle the case where the target is trivial
		check_finished(i);
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
		if (m_pf_states[i] == ProofState::UNFINISHED)
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
			ATP_SEARCH_ASSERT(m_proofs[i].has_value());
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
		phxarg::arg1 == ProofState::UNFINISHED);
}


void IterativeDeepeningSolver::step_proof(size_t i, size_t n)
{
	auto& st = m_stacks[i];

	if (st.empty())
		return;

	for (size_t t = 0; t < n; t++)
	{
		// expand an element element if depth limit not reached.
		if (st.size() < m_cur_depth_limits[i])
		{
			expand_next(i);
		}
		// else pop from the stack and advance
		else
		{
			trim_expansion(i);

			// now we need to pay particular attention to the size of
			// the stack. This is because it gives us an indication
			// of what we need to do next.

			if (st.size() > 1)
			{
				// next step, explore a new child
				st.back().m_idx++;

				ATP_SEARCH_ASSERT(st.back().m_idx <
					st.back().m_stmts->size());
			}
			else if (st.size() == 1)
			{
				// increment the depth limit by 1 as we have fully
				// exhausted the previous depth (and have returned
				// to the root).
				m_cur_depth_limits[i]++;
			}
			// else if st.empty() we are done (given up)
		}

		// update max mem count for this proof
		const auto mem_count = count_mem(i);
		if (mem_count > m_max_mem[i])
			m_max_mem[i] = mem_count;

		if (check_finished(i))
			return;
	}
}


void IterativeDeepeningSolver::expand_next(size_t i)
{
	ATP_LOGIC_PRECOND(i < m_stacks.size());
	auto& st = m_stacks[i];

	ATP_SEARCH_PRECOND(st.size() < m_cur_depth_limits[i]);
	ATP_SEARCH_PRECOND(st.back().m_idx < st.back().m_stmts->size());

	do
	{
		auto expand_candidate = logic::from_statement(
			st.back().m_stmts->at(st.back().m_idx)
		);

#ifdef ATP_SEARCH_DEFENSIVE
		// if we encounter a true statement, then our detection of true
		// statements has clearly failed!
		ATP_SEARCH_ASSERT(m_kernel->get_form(expand_candidate)[0]
			!= logic::StmtForm::CANONICAL_TRUE);
#endif

		auto succs = m_kernel->succs(expand_candidate);

		// we haven't vectorised this
		ATP_SEARCH_ASSERT(succs.size() == 1);

		// we have expanded another node:
		m_num_node_exps[i]++;

		auto succs_to_add = filter_succs(succs[0]);

		// only create a new stack frame if there were any
		// successors
		if (succs_to_add->size() > 0)
		{
			// push new stack frame
			st.push_back(StackFrame{
				succs_to_add, 0
				});

			// exit, we are done
			return;
		}
		else
		{
			// check out the next node we could expand (if any)
			++st.back().m_idx;
		}

	} while (st.back().m_idx < st.back().m_stmts->size());

	// if we got here then the proof has failed because there were
	// no possible further expansions

	m_stacks[i].clear();
	m_pf_states[i] = ProofState::NO_PROOF;
}


void IterativeDeepeningSolver::trim_expansion(size_t i)
{
	ATP_LOGIC_PRECOND(i < m_stacks.size());
	auto& st = m_stacks[i];
	auto st_iter = st.rbegin();

	// find the next location where we can expand from
	// (this is effectively going back up the stack to
	// find a place with a new sibling node we haven't
	// seen before)
	while (st_iter != st.rend()
		&& st_iter->m_idx + 1 == st_iter->m_stmts->size())
	{
		st_iter++;
	}

	// if we found something we haven't finished exploring yet...
	if (st_iter != st.rend())
	{
		// delete all the stuff we've just explored:
		st.erase(st_iter.base(), st.end());
	}
	else
	{
		// finished an iteration, time to increase the
		// depth limit!

		if (m_cur_depth_limits[i] < m_max_depth)
		{
			// erase everything except the root node
			st.erase(std::next(st.begin()), st.end());

			ATP_SEARCH_ASSERT(st.size() == 1);
		}
		else
		{
			// Oh dear! We have reached the ultimate
			// depth limit without finding a proof.
			// There is now nothing we can do, so just
			// clear the stack to indicate this.
			st.clear();
		}
	}
}


bool IterativeDeepeningSolver::check_finished(size_t i)
{
	ATP_LOGIC_PRECOND(i < m_stacks.size());
	auto& st = m_stacks[i];

	auto forms = m_kernel->get_form(st.back().m_stmts);

	// if any of the current final states are canonically true
	// then we have a proof
	auto iter = std::find(forms.begin(), forms.end(),
		logic::StmtForm::CANONICAL_TRUE);
	if (iter != forms.end())
	{
		const auto idx = std::distance(forms.begin(), iter);
		// setting this makes it easier to build the proof below
		// (this is fine because we're about to clear the stack
		// anyway).
		st.back().m_idx = idx;

		// build proof
		std::vector<logic::StatementArrayPtr> proof_steps;
		proof_steps.reserve(st.size());
		for (auto st_frame : st)
		{
			proof_steps.push_back(st_frame.m_stmts->slice(
				st_frame.m_idx, st_frame.m_idx + 1));
		}
		m_proofs[i] = logic::concat(proof_steps);

		m_pf_states[i] = ProofState::DONE_TRUE;
		st.clear();

		return true;
	}

	return false;
}


logic::StatementArrayPtr IterativeDeepeningSolver::filter_succs(
	logic::StatementArrayPtr succs) const
{
	// unfortunately the statement arrays don't have a filter
	// function so we'll have to do with this:

	if (succs->size() == 0)
		return succs;  // nothing to do

	auto forms = m_kernel->get_form(succs);

	std::vector<logic::StatementArrayPtr> filtered;
	filtered.reserve(succs->size());

	for (size_t i = 0; i < succs->size(); ++i)
	{
		if (forms[i] != logic::StmtForm::CANONICAL_FALSE)
			filtered.push_back(succs->slice(i, i + 1));
	}

	return logic::concat(filtered);
}


size_t IterativeDeepeningSolver::count_mem(size_t i) const
{
	size_t count = 0;

	for (const auto& st_frame : m_stacks[i])
	{
		count += st_frame.m_stmts->size();
	}

	return count;
}


}  // namespace search
}  // namespace atp


