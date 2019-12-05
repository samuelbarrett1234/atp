#pragma once


// Author: Samuel Barrett


#include <memory>
#include "Process.h"
#include "LockManager.h"



namespace atp
{


/// <summary>
/// The process manager is a class designed to handle concurrency
/// and resource locking automatically. To use it, you can submit
/// processes to it, and you can also set up several threads to
/// run the process manager in.
/// This class is thread-safe.
/// </summary>
class ProcessManager
{
public:
	ProcessManager();
	~ProcessManager();

	/// <summary>
	/// Calling 'run' will cause the calling thread to block
	/// indefinitely until the process manager finishes all
	/// processes. This function continually executes processes
	/// until all are finished.
	/// </summary>
	void run();


	/// <summary>
	/// Run a single step of a single process. This translates
	/// to a single init/update/release call.
	/// </summary>
	void run_one();


	/// <summary>
	/// Abort all processes. This will cause any threads running
	/// run() to exit the function.
	/// </summary>
	void abort_all();


	/// <summary>
	/// Add another process to the process manager.
	/// Will be added to the queue for running.
	/// </summary>
	/// <param name="pProc">A pointer to the process object.</param>
	/// <param name="dependsOn">
	/// An optional ID; if given, the process manager will only execute
	/// the process once a process with this ID has finished.
	/// PRECONDITION: pProc->get_id() != dependsOn (if both are provided).
	/// Edge cases:
	/// If such a process ID does not exist, either because it has already
	/// finished or hasn't been added yet, then the process will just be
	/// executed as normal.
	/// </param>
	/// <remarks>
	/// If you are trying to add a dependency graph (DAG) of processes, then
	/// you must post them in a topologically-sorted order. The dependency
	/// management of this function has been set up to make it impossible to
	/// create cyclic dependencies (as when the dependsOn ID does not exist,
	/// the dependency is ignored!)
	/// </remarks>
	void post(ProcessPtr pProc, ProcessID dependsOn = ProcessID());

private:
	LockManager m_lockMgr;
};


} // namespace atp


