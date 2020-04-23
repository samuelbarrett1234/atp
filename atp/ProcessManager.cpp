/*
\file

\author Samuel Barrett

*/


#include "ProcessManager.h"
#include "ATP.h"


void ProcessManager::add(ProcessPtr p_proc)
{
	switch (p_proc->state())
	{
	case ProcessState::AWAITING_LOCK:
		push_awaiting_lock(std::move(p_proc));
		break;
	case ProcessState::RUNNING:
		push_running(std::move(p_proc));
		break;
	case ProcessState::DONE:
		break;  // do nothing and let the process destruct!
	}
}


void ProcessManager::commit_thread(std::ostream& output)
{
	// every N iterations we will check for procs awaiting locks
	const size_t N = 5;
	size_t i = 0;  // invariant: i < N

	while (!done())
	{
		ProcessPtr p_proc;

		if (++i == N)
		{
			i = 0;

			p_proc = pop_awaiting_lock();

			ATP_ASSERT(p_proc->state() ==
				ProcessState::AWAITING_LOCK);

			p_proc->try_acquire_lock();
		}
		else
		{
			p_proc = pop_running();

			ATP_ASSERT(p_proc->state() ==
				ProcessState::RUNNING);

			p_proc->run_step();
		}

		// log process stuff
		p_proc->dump_log(output);

		// put the process back in the queues if necessary.
		switch (p_proc->state())
		{
		case ProcessState::AWAITING_LOCK:
			push_awaiting_lock(std::move(p_proc));
			break;
		case ProcessState::RUNNING:
			push_running(std::move(p_proc));
			break;
		case ProcessState::DONE:
			break;  // do nothing, let the process destruct!
		}
	}
}


bool ProcessManager::done() const
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	return m_awaiting_lock.empty() && m_running.empty();
}


ProcessPtr ProcessManager::pop_running()
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto p_result = std::move(m_running.front());
	m_running.pop_front();
	return p_result;
}


ProcessPtr ProcessManager::pop_awaiting_lock()
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	auto p_result = std::move(m_awaiting_lock.front());
	m_awaiting_lock.pop_front();
	return p_result;
}


void ProcessManager::push_running(ProcessPtr p_proc)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	m_running.push_back(std::move(p_proc));
}


void ProcessManager::push_awaiting_lock(ProcessPtr p_proc)
{
	std::scoped_lock<std::mutex> lock(m_mutex);

	m_awaiting_lock.push_back(std::move(p_proc));
}


