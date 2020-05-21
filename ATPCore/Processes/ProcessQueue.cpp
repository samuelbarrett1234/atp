/*
\file

\author Samuel Barrett

*/


#include "ProcessQueue.h"


namespace atp
{
namespace core
{


ProcessQueue::ProcessQueue() :
	// every N iterations we will check waiting procs
	N(10),
	i(0)
{ }


void ProcessQueue::push(ProcessPtr p_proc)
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


ProcessPtr ProcessQueue::try_pop()
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


bool ProcessQueue::done() const
{
	std::scoped_lock<std::mutex> lk(m_mutex);

	return m_running.empty() && m_waiting.empty() &&
		m_executing.empty();
}


boost::tuple<size_t, size_t, size_t> ProcessQueue::size() const
{
	std::scoped_lock<std::mutex> lk(m_mutex);

	return boost::make_tuple(m_running.size(),
		m_waiting.size(),
		m_executing.size());
}


}  // namespace core
}  // namespace atp


