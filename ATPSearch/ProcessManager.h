#pragma once


// Author: Samuel Barrett


#include "Process.h"


namespace atpsearch
{


/// <summary>
/// This is the main class of this library, which manages
/// the execution of many processes and resources. It is
/// ran by dedicating one or more threads to process execution,
/// and one or more threads to I/O. There must be at least
/// one thread running each.
/// </summary>
class ProcessManager
{
public:

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

private:
};


} // namespace atpsearch


