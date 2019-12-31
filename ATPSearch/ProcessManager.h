#pragma once


// Author: Samuel Barrett


#include "Process.h"
#include "ProcessScheduler.h"
#include "LockManager.h"
#include "ResourceOperation.h"
#include "ResourceOperationScheduler.h"
#include <map>


namespace atpsearch
{


/// <summary>
/// This is the main class of this library, which manages
/// the execution of many processes and resources. It is
/// ran by dedicating one or more threads to process execution,
/// and one or more threads to I/O. There must be at least
/// one thread running each.
/// </summary>
class ATP_API ProcessManager
{
public:
	ProcessManager(ProcessSchedulerType processSchedulerType,
		ResourceOperationSchedulerType resOpSchedulerType,
		LockManagementType lkMgmtType);

	/// <summary>
	/// Calling this function blocks the calling thread
	/// until the process manager stops. The thread will
	/// repeatedly perform process ticks (i.e. do process
	/// updates).
	/// </summary>
	void run_processes();

	/// <summary>
	/// Calling this function blocks the calling thread
	/// until the process manager stops. The thread will
	/// repeatedly handle IO requests from the process
	/// ticks. This thread should spend most of its time
	/// sleeping.
	/// </summary>
	void run_io();

	/// <summary>
	/// Add a new process to the system. The process manager
	/// assumes control of the memory of this process. This
	/// function is thread-safe, and can be called from inside
	/// another process, or from another thread entirely.
	/// I.e. this function is thread-safe and never blocks.
	/// </summary>
	void post(ProcessPtr pProc);

	/// <summary>
	/// This will stop all threads from executing any processes,
	/// cancel any IO operations, and then all processes will be
	/// aborted, and then all processes will be cleared (returning
	/// this process manager back to its initial state). All worker
	/// threads will be allowed to exit, and should do so shortly
	/// after they have helped abort all processes.
	/// </summary>
	void stop();
	
	/// <summary>
	/// Register a resource with the process manager. This allows
	/// processes to access it.
	/// Precondition: the ID of this resource has not already been
	/// registered.
	/// </summary>
	/// <param name="pRes">A pointer to the resource object to add.</param>
	void register_resource(ResourcePtr pRes);

private:
	/// <summary>
	/// Add a list of resource operations for a given process. This
	/// will mean that the process will be blocked until they are
	/// complete.
	/// </summary>
	/// <param name="worker_id">The ID of the worker (process) which the resource operations belong to.</param>
	/// <param name="ops">A list of resource operations.</param>
	void add_resource_operations(size_t worker_id, const ResourceOperations& ops);

	/// <summary>
	/// Determine if this process is waiting on resource operations
	/// to be completed. If so, the process must be blocked until
	/// they are finished.
	/// </summary>
	/// <param name="worker_id">The ID of the worker (process) to check.</param>
	/// <returns>True if waiting, false if ready.</returns>
	bool proc_waiting_on_res_op(size_t worker_id) const;

	/// <summary>
	/// Undo all resource operations of a worker.
	/// Precondition: the worker has been aborted by the lock manager,
	/// which means that I/O threads cannot pick up new tasks for this
	/// worker. This function will wait until all I/O threads have
	/// finished working on this process's stuff, then will undo them
	/// all.
	/// </summary>
	/// <param name="worker_id">The ID of the worker (process) to undo.</param>
	void undo_res_ops(size_t worker_id);

private:
	/// <summary>
	/// This is some data associated with an active process.
	/// The timestamp is used to prioritise processes. It is
	/// not a "true" timestamp in the sense that it is derived
	/// from the current system time - it is just incremented
	/// by 1 for each subsequent process.
	/// </summary>
	struct ProcMetaData  // WIP
	{
		size_t timestamp;
		bool bReadyToDestroy;
	};

	struct IOOperationMetaData  // WIP
	{
		size_t id;
		size_t dependsOn; // Another IO op ID, or -1 if doesn't depend.
		ResourceOperation op;
	};

private:
	bool m_bStop;
	ProcessSchedulerPtr m_pProcessScheduler;
	ResourceOperationSchedulerPtr m_pResOpScheduler;
	LockManagerPtr m_pLkMgr;

	size_t m_NextTimestamp;  // Starts at 0, increments by 1 for every process
	std::map<const IProcess*, ProcMetaData> m_MetaData;  // metadata for each process
};


} // namespace atpsearch


