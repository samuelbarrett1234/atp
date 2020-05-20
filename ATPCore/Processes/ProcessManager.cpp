/*
\file

\author Samuel Barrett

*/


#include "ProcessManager.h"


namespace atp
{
namespace core
{


void ProcessManager::add(ProcessPtr p_proc)
{
	m_queue.push(std::move(p_proc));
}


void ProcessManager::commit_thread()
{
	// we will run processes N times every time we pop them from the
	// queue
	static const size_t N = 50;

	while (!m_queue.done())
	{
		// may be null!!
		ProcessPtr p_proc = m_queue.try_pop();

		// try again if we didn't get one:
		if (p_proc == nullptr)
			continue;

		ATP_CORE_ASSERT(!p_proc->done());
		for (size_t i = 0; i < N && !p_proc->done(); ++i)
		{
			p_proc->run_step();
		}

		// always put the process back onto the queue, regardless
		// of its current state
		m_queue.push(std::move(p_proc));
	}
}


size_t ProcessManager::num_procs_running() const
{
	return m_queue.size();
}


ProcessManager::ProcQueue::ProcQueue() :
	// every N iterations we will check waiting procs
	N(10),
	i(0)
{ }


void ProcessManager::ProcQueue::push(ProcessPtr p_proc)
{
	ATP_CORE_PRECOND(p_proc != nullptr);
	std::scoped_lock<std::mutex> lk(m_mutex);

	// remove this from the executing set, if it was in there
	m_executing.erase(p_proc.get());

	if (!p_proc->done())
	{
		if (p_proc->waiting())
		{
			m_waiting.emplace(std::move(p_proc));
		}
		else
		{
			m_running.emplace(std::move(p_proc));
		}
	}
}


ProcessPtr ProcessManager::ProcQueue::try_pop()
{
	std::scoped_lock<std::mutex> lk(m_mutex);

	if (m_running.empty() && m_waiting.empty())
		return nullptr;

	i = (i + 1) % N;

	if ((i == 0 && !m_waiting.empty()) || m_running.empty())
	{
		auto result = std::move(m_waiting.front());
		m_waiting.pop();
		m_executing.insert(result.get());
		return result;
	}
	else
	{
		auto result = std::move(m_running.front());
		m_running.pop();
		m_executing.insert(result.get());
		return result;
	}
}


bool ProcessManager::ProcQueue::done() const
{
	std::scoped_lock<std::mutex> lk(m_mutex);

	return m_running.empty() && m_waiting.empty() &&
		m_executing.empty();
}


size_t ProcessManager::ProcQueue::size() const
{
	std::scoped_lock<std::mutex> lk(m_mutex);

	return m_running.size() + m_waiting.size() +
		m_executing.size();
}


}  // namespace core
}  // namespace atp


