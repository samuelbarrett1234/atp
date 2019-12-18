#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "Process.h"


namespace atpsearch
{


/// <summary>
/// This class has the responsibility of distributing the work,
/// not executing it. It is basically a central, thread-safe
/// queue.
/// </summary>
class ATP_API IScheduler
{
public:
	virtual ~IScheduler() = default;

	/// <summary>
	/// Add a process to the queue.
	/// This function is thread safe.
	/// Precondition: pProc != nullptr
	/// </summary>
	/// <param name="pProc">The process to add.</param>
	virtual void push(ProcessPtr pProc) = 0;

	/// <summary>
	/// Get the next process for the calling thread.
	/// This function is thread safe.
	/// For example, this function could pop from the
	/// central queue, or use the current thread ID,
	/// etc.
	/// Postcondition: returned value is nullptr iff
	/// there is no available work for this thread.
	/// If a process is returned, then it is removed
	/// from the scheduler, and it is the thread's
	/// responsibility to push it again. (!!!)
	/// </summary>
	/// <returns>A pointer to a process which needs work iff there is work available.</returns>
	virtual ProcessPtr pop() = 0;
};


} // namespace atpsearch


