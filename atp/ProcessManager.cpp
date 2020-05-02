/*
\file

\author Samuel Barrett

*/


#include "ProcessManager.h"
#include "ATP.h"


void ProcessManager::add(ProcessPtr p_proc)
{
	if (!p_proc->done())
	{
		if (p_proc->waiting())
			push_waiting(std::move(p_proc));
		else
			push_running(std::move(p_proc));
	}
	// else do nothing and let the process destruct!
}


void ProcessManager::commit_thread(std::ostream& output)
{
	// every N iterations we will check waiting procs
	const size_t N = 5;
	size_t i = 0;  // invariant: i < N

	while (!done())
	{
		ProcessPtr p_proc;

		i = (i + 1) % N;

		if (i == 0)
		{
			p_proc = pop_waiting();

			ATP_ASSERT(p_proc->waiting());
		}
		else
		{
			p_proc = pop_running();

			ATP_ASSERT(!p_proc->waiting());
		}

		ATP_ASSERT(!p_proc->done());
		p_proc->run_step();

		// log process stuff
		p_proc->dump_log(output);

		// put the process back in the queues if necessary.
		add(std::move(p_proc));
	}
}


bool ProcessManager::done() const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	return m_waiting.empty() && m_running.empty();
}


ProcessPtr ProcessManager::pop_running()
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto p_result = std::move(m_running.front());
	m_running.pop();
	return p_result;
}


ProcessPtr ProcessManager::pop_waiting()
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto p_result = std::move(m_waiting.front());
	m_waiting.pop();
	return p_result;
}


void ProcessManager::push_running(ProcessPtr p_proc)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	m_running.emplace(std::move(p_proc));
}


void ProcessManager::push_waiting(ProcessPtr p_proc)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	m_waiting.emplace(std::move(p_proc));
}


