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
	ProcessManager(ProcessSchedulerType schedulerType, LockManagementType lkMgmtType);

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


private:
	/// <summary>
	/// This is some data associated with an active process.
	/// The timestamp is used to prioritise processes. It is
	/// not a "true" timestamp in the sense that it is derived
	/// from the current system time - it is just incremented
	/// by 1 for each subsequent process.
	/// </summary>
	struct ProcMetaData
	{
		size_t timestamp;
		bool bReadyToDestroy;
	};

	struct IOOperationMetaData
	{
		size_t id;
		size_t dependsOn; // Another IO op ID, or -1 if doesn't depend.
		ResourceOperation op;
	};

private:
	bool m_bStop;
	ProcessSchedulerPtr m_pProcessScheduler;
	LockManagerPtr m_pLkMgr;

	size_t m_NextTimestamp;  // Starts at 0, increments by 1 for every process
	std::map<const IProcess*, ProcMetaData> m_MetaData;  // metadata for each process
};


} // namespace atpsearch


