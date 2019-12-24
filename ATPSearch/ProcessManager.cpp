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
		auto meta = m_MetaData.at(pProc.get());
		const size_t worker_id = meta.timestamp;  // use timestamp as ID (lower timestamp => older process => higher priority)

		// If the process is ready to be destroyed, we can do so now:
		if (meta.bReadyToDestroy)  // TODO: CHECK IF HAS ANY WAITING IO OPERATIONS
		{
			m_pLkMgr->remove_worker(worker_id);
			continue;  // this calls the destructor of pProc, which destructs our process.
		}

		// Now, check its lock status:

		WorkerStatus worker_status;
		auto _freeze_lock = m_pLkMgr->get_status_freeze_if_ready(worker_id, &worker_status);

		switch (worker_status)
		{
		case WorkerStatus::READY:
		{
			const auto procStat = pProc->tick();
			// TODO: handle different process statuses (i.e.
			// whether the process finished or not, and whether
			// or not there are any resource operations.)

			// Not finished & no res-ops: just push onto scheduler
			// Not finished & res-ops: push onto scheduler but add
			// the resource operations to however they are managed.
			// Finished & no res-ops: greedily delete process
			// Finished & res-ops: unfortunately still need to push
			// onto the scheduler because the res-op may depend on
			// data held in the process (so need to delete as soon
			// as the scheduler next pops it).
		}
			break;

		case WorkerStatus::FAILED:
			// TODO: before aborting, check to see if there are any I/O operations
			// that this process is waiting on, and if so, block until they are done.
			pProc->abort();
			m_pLkMgr->remove_worker(worker_id);  // release all locks
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


