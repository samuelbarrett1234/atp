#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include <memory>


namespace atpsearch
{


class ILockManager;


/// <summary>
/// These are the states a worker (i.e. a process)
/// can be in.
/// </summary>
enum class WorkerStatus
{
	READY,  // owns all the locks it has requested
	BLOCKED,  // is waiting on other processes which have the lock
	FAILED  // needs to be aborted.
};


/// <summary>
/// There are two types of lock: eXclusive locks
/// and Shared locks.
/// </summary>
enum class LockType
{
	XLOCK,
	SLOCK
};


/// <summary>
/// This is a convenience class for automatic freezing and unfreezing
/// of a worker during a tick() operation. Its only job is to unfreeze
/// the worker in the destructor (it does NOT freeze it in the constructor,
/// that is the job of the lock manager implementation.)
/// Note: this doesn't necessarily have to lock anything, if we use the
/// constructor overload which only takes one argument.
/// Note to the library user: you should never have to construct one of
/// these objects, they need only be obtained as the return value of a lock
/// manager function.
/// </summary>
class ATP_API FreezeLock
{
public:
	FreezeLock(ILockManager& lkmgr);
	FreezeLock(size_t worker_id, ILockManager& lkmgr);
	~FreezeLock();

private:
	bool bIsALock;
	const size_t worker_id;
	ILockManager& lkmgr;
};


/// <summary>
/// Lock managers are for ensuring only certain processes
/// get access to resources. They are abstracted away from
/// the interface of a process - all they need is some process
/// identifier. There are many ways of implementing this; see
/// two-phase-locking, wait-die, wound-wait.
/// IMPORTANT: the worker IDs are precisely their priorities.
/// Lower ID means higher priority!
/// </summary>
class ATP_API ILockManager
{
public:
	virtual ~ILockManager() = default;

	/// <summary>
	/// This function is thread-safe.
	/// Call this when a worker requests access to a specific
	/// resource. This can either result in success or failure.
	/// Use XLOCK for read-write access and SLOCK for read-only
	/// (but shared) access.
	/// Warning: this function blocks if this request causes a
	/// state change in a worker which is currently frozen (e.g.
	/// in wound-wait, an older worker may request a resource
	/// currently held by a younger worker which is currently
	/// undergoing a tick() - as a result we must wait for the
	/// tick() function to return before aborting it.)
	/// PRECONDITION: the worker_id must NOT be frozen.
	/// </summary>
	/// <param name="worker_id">The ID of the worker requesting the resource.</param>
	/// <param name="resource_id">The ID of the resource being requested.</param>
	/// <param name="lk_type">The type of lock being requested.</param>
	virtual void request(size_t worker_id, size_t resource_id, LockType lk_type) = 0;

	/// <summary>
	/// Get the status of a worker, while simultaneously freezing
	/// the worker if and only if its status is READY. This operation
	/// is "atomic" in the sense that it is NOT possible for the
	/// worker to be in the ready state, then frozen, and then for
	/// another thread to change its state (as one would hope!)
	/// </summary>
	/// <param name="worker_id">The ID of the worker (process) to check.</param>
	/// <param name="pOutStatus">
	/// A pointer to a location to write the status to. If nullptr then nothing is written.
	/// </param>
	/// <returns>
	/// A lock object which freezes the worker as long as it is alive, if the status is ready.
	/// If the worker was not ready then this object is effectively invalid, and doesn't do anything.
	/// </returns>
	virtual FreezeLock get_status_freeze_if_ready(size_t worker_id, WorkerStatus* pOutStatus) = 0;
	
	/// <summary>
	/// This function is thread-safe.
	/// Call this when a worker finishes (and no longer needs
	/// any of its resources) or after being aborted (in which
	/// case all of its locks need to be released).
	/// Calling this function is rather crucial!
	/// PRECONDITION: the worker_id must not be frozen.
	/// </summary>
	/// <param name="worker_id">The ID of the worker to release all the locks of.</param>
	virtual void remove_worker(size_t worker_id) = 0;

protected:  // only the freeze lock needs to call this
	friend class FreezeLock;

	/// <summary>
	/// This function is thread-safe.
	/// Undoes the result of freeze_worker().
	/// </summary>
	/// <param name="worker_id">The ID of the worker to unfreeze.</param>
	virtual void unfreeze_worker(size_t worker_id) = 0;
};


typedef std::unique_ptr<ILockManager> LockManagerPtr;


/// <summary>
/// Different ways of managing the lock table.
/// These are basically necessary to resolve deadlocks.
/// </summary>
enum class LockManagementType
{
	// An older process will wait for a younger one if it is last to acquire a lock,
	// A younger process will abort if it tries to acquire a lock held by an older process
	WAIT_DIE,
	// An older process will take priority over a younger one if it tries to acquire its lock,
	// A younger process will wait for an older one if it tries to acquire its lock
	WOUND_WAIT
};


/// <summary>
/// Allocate a lock manager implementing the given type.
/// </summary>
ATP_API LockManagerPtr create_lock_manager(LockManagementType type);


} // namespace atpsearch


