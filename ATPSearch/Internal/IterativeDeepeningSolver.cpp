#include "IterativeDeepeningSolver.h"
#include <boost/timer/timer.hpp>


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
	// 1 for the root node:
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
	}
}


void IterativeDeepeningSolver::step(size_t n)
{
	ATP_SEARCH_PRECOND(n > 0);

	boost::timer::cpu_timer timer;
	timer.stop();  // timer starts by default when constructed

	for (size_t i = 0; i < m_stacks.size(); i++)
	{
		// start the timer individually for each proof
		// .start() resets timer to zero whereas .resume()
		// starts from the time we left off.
		timer.start();

		step_proof(i, n);

		// add the total time to our tracker:
		timer.stop();
		auto time = timer.elapsed();
		m_agg_time[i] += time.user + time.system;
	}
}


void IterativeDeepeningSolver::clear()
{
	m_targets.reset();
	m_stacks.clear();
	m_proofs.clear();
	m_agg_time.clear();
	m_num_node_exps.clear();
}


bool IterativeDeepeningSolver::any_proof_not_done() const
{
	return std::any_of(m_proofs.begin(),
		m_proofs.end(),
		[](boost::optional<logic::StatementArrayPtr> o)
		{ return o.has_value(); });
}


void IterativeDeepeningSolver::step_proof(size_t i, size_t n)
{
	for (size_t t = 0; t < n; t++)
	{
		auto& st = m_stacks[i];

		// expand an element element if depth limit not reached.
		if (st.size() < m_cur_depth_limits[i])
		{
			expand_next(i, st);
		}
		// else pop from the stack and advance
		else
		{
			trim_expansion(i, st);

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
	}
}


void IterativeDeepeningSolver::expand_next(size_t i,
	std::list<StackFrame>& st)
{
	ATP_SEARCH_PRECOND(st.size() < m_cur_depth_limits[i]);

	auto succs = m_kernel->succs(logic::from_statement(
		st.back().m_stmts->at(st.back().m_idx)
	));

	// we haven't vectorised this
	ATP_SEARCH_ASSERT(succs.size() == 1);

	// we have expanded another node:
	m_num_node_exps[i]++;

	// push successors
	st.push_back(StackFrame{
		succs[0], 0
		});
}


void IterativeDeepeningSolver::trim_expansion(size_t i,
	std::list<StackFrame>& st)
{
	auto st_iter = st.rbegin();
	auto st_last = st_iter;

	// find the next location where we can expand from
	// (this is effectively going back up the stack to
	// find a place with a new sibling node we haven't
	// seen before
	while (st_iter != st.rend()
		&& st_iter->m_idx + 1 == st_iter->m_stmts->size())
	{
		st_last = st_iter;
		st_iter++;
	}

	// if we found something we haven't explored yet...
	if (st_iter != st.rend())
	{
		// delete all the stuff we've just explored:
		st.erase(st_last.base(), st.rbegin().base());

		// expected number of elements left
		ATP_SEARCH_ASSERT(st.size() ==
			std::distance(st_iter, st.rend()));
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


}  // namespace search
}  // namespace atp


