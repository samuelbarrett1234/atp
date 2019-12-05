#pragma once


// Author: Samuel Barrett


#include <memory>
#include <boost/optional.hpp>


namespace atp
{


class LockBroker;


class IProcess;
typedef std::unique_ptr<IProcess> ProcessPtr;


/// <summary>
/// Not all processes need an ID, but those that
/// do are integers.
/// </summary>
typedef boost::optional<size_t> ProcessID;


/// <summary>
/// A "process" represents a computation which is
/// typically intended to exist over some time,
/// and to be executed in several steps, while
/// requesting locks to various resources, etc.
/// </summary>
class IProcess
{
public:
	virtual ~IProcess() = default;


	/// <summary>
	/// This is called when the process is first
	/// executed. It gives the process an opportunity
	/// to initialise resources and acquire locks.
	/// </summary>
	/// <param name="lockBroker">The lock broker, to request locks from.</param>
	virtual void init(LockBroker& lockBroker) = 0;


	/// <summary>
	/// This is the main process function; it is where
	/// the process should do a unit of computation.
	/// It should not block for too long, as this is
	/// unfair to other procseses.
	/// </summary>
	virtual void update(LockBroker& lockBroker) = 0;


	/// <summary>
	/// Determine if the process has finished.
	/// If it has, release() will be called and
	/// the process will be destroyed.
	/// </summary>
	/// <returns>True if the process has finished, false if not.</returns>
	virtual bool is_finished() const = 0;


	/// <summary>
	/// This is called by the process manager when it
	/// decides to abort this process. This is called
	/// before release(). Here, the process should undo
	/// any necessary changes it has made in update().
	/// It is important the process holds on to any locks
	/// it has acquired to ensure that it can perform
	/// this abort.
	/// </summary>
	virtual void abort() = 0;


	/// <summary>
	/// This is ALWAYS called, and is the last thing to
	/// be called on a process, before it dies. Perform
	/// any cleanup. After this is called, the process's
	/// locks are released as well.
	/// You also have the option to return a child process
	/// to be posted to the process manager automatically.
	/// </summary>
	/// <returns>
	/// (Optional) a pointer to a new process to post.
	/// Return null if you don't want to do this.
	/// </returns>
	virtual ProcessPtr release() = 0;


	/// <summary>
	/// Get the process ID (you can return ProcessID() if
	/// this proc doesn't have an ID).
	/// </summary>
	/// <returns>The process ID.</returns>
	virtual ProcessID get_id() const = 0;
};


} // namespace atp


