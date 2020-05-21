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
#ifdef _DEBUG
	static const size_t N = 1;
#else
	static const size_t N = 10;
#endif

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


}  // namespace core
}  // namespace atp


