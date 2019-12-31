#include "ProcessManager.h"
#include "Error.h"
#include <thread>


// Author: Samuel Barrett


static size_t get_this_thread_id()
{
	// Need to hash the thread IDs to keep the program platform-agnostic,
	// as thread IDs are not guaranteed to be integers.
	// See https://stackoverflow.com/questions/7432100/how-to-get-integer-thread-id-in-c11
	return std::hash<std::thread::id>{}(std::this_thread::get_id());
}


namespace atpsearch
{


void ProcessManager::run_processes()
{
	while (!m_bStop)
	{
		// Firstly, obtain a process:

		ProcessPtr pProc;
		while (!pProc)
		{
			pProc = m_pProcessScheduler->pop(get_this_thread_id());
		}

		// Now get metadata:
		auto& meta = m_MetaData.at(pProc.get());
		const size_t worker_id = meta.timestamp;  // use timestamp as ID (lower timestamp => older process => higher priority)

		if (proc_waiting_on_res_op(worker_id))
		{
			// Try again later:
			m_pProcessScheduler->push(std::move(pProc), get_this_thread_id());
			continue;  // this calls the destructor of pProc, which destructs our process.
		}

		// If the process is ready to be destroyed, we can do so now:
		if (meta.bReadyToDestroy)
		{
			m_pLkMgr->remove_worker(worker_id);
			continue;  // this calls the destructor of pProc, which destructs our process.
		}

		// Otherwise, check its lock status:

		WorkerStatus worker_status;
		auto _freeze_lock = m_pLkMgr->get_status_freeze_if_ready(worker_id, &worker_status);

		switch (worker_status)
		{
		case WorkerStatus::READY:
		{
			const auto proc_stat = pProc->tick();

			// This will not request any locks immediately - this will
			// happen "as we go along" in the I/O threads, requesting
			// locks as we use different resources.
			add_resource_operations(worker_id, proc_stat.ops);

			if (proc_stat.bFinished)
			{
				if (proc_stat.ops.empty())
				{
					// Can finish immediately
					m_pLkMgr->remove_worker(worker_id);  // release all locks
					pProc.reset();  // calls destructor
				}
				else
				{
					// Cannot finish immediately - finish next time we check it
					meta.bReadyToDestroy = true;
					m_pProcessScheduler->push(std::move(pProc), get_this_thread_id());
				}
			}
			else
			{
				// Push to continue ticking() next time we come across it
				m_pProcessScheduler->push(std::move(pProc), get_this_thread_id());
			}
		}
			break;

		case WorkerStatus::FAILED:
			undo_res_ops(worker_id);
			pProc->abort();
			m_pLkMgr->remove_worker(worker_id);
			// Try again later:
			// TODO: add a random time delay for restart?
			m_pProcessScheduler->push(std::move(pProc), get_this_thread_id());
			break;

		case WorkerStatus::BLOCKED:
			// Try again later:
			m_pProcessScheduler->push(std::move(pProc), get_this_thread_id());
			break;

		default:
			ATP_CHECK_INVARIANT(false, "Invalid lock manager status returned for process.");
		}
	}
}


} // namespace atpsearch


