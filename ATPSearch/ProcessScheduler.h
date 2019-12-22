#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "Process.h"
#include <memory>


namespace atpsearch
{


/// <summary>
/// This class has the responsibility of distributing the work,
/// not executing it. It is basically a central, thread-safe
/// queue for processes.
/// </summary>
class ATP_API IProcessScheduler
{
public:
	virtual ~IProcessScheduler() = default;

	/// <summary>
	/// Add a process to the queue.
	/// This function is thread safe.
	/// Precondition: pProc != nullptr
	/// </summary>
	/// <param name="pProc">The process to add.</param>
	/// <param name="thread_id">
	/// The ID of the calling thread (extracted as a parameter
	/// to allow this class to be unit tested - it suffices
	/// to pass a hash of std::this_thread::get_id()).
	/// </param>
	virtual void push(ProcessPtr pProc, size_t thread_id) = 0;

	/// <summary>
	/// Get the next process for the calling thread.
	/// This function is thread safe.
	/// For example, this function could pop from the
	/// central queue, or use the current thread ID,
	/// etc.
	/// Postcondition: returned value is nullptr iff
	/// there is no available work for this thead. Exactly
	/// when this happens depends on the scheduler implementation.
	/// If a process is returned, then it is removed
	/// from the scheduler, and it is the thread's
	/// responsibility to push it again. (!!!)
	/// </summary>
	/// <param name="thread_id">
	/// The ID of the calling thread (extracted as a parameter
	/// to allow this class to be unit tested - it suffices
	/// to pass a hash of std::this_thread::get_id()).
	/// </param>
	/// <returns>A pointer to a process which needs work iff there is work available.</returns>
	virtual ProcessPtr pop(size_t thread_id) = 0;
};


typedef std::unique_ptr<IProcessScheduler> ProcessSchedulerPtr;


/// <summary>
/// Different types of scheduler algorithm for processes
/// </summary>
enum class ProcessSchedulerType
{
	// When a thread runs out of processes it will steal
	// one from another thread at random (hence if the
	// queue for ANY thread is nonempty, ANY OTHER thread
	// will always be able to find work.)
	WORK_STEALING
};


ATP_API ProcessSchedulerPtr create_process_scheduler(ProcessSchedulerType type);


} // namespace atpsearch


