#pragma once


// Author: Samuel Barrett


#include "Utility.h"


namespace atpsearch
{


/// <summary>
/// A request for a lock can either pass (resulting
/// in either access to the resource or being added
/// to a waiting list) or it can fail (meaning the
/// process needs to be aborted to avoid deadlock).
/// </summary>
enum class LockRequestResult
{
	PASSED,
	FAILED
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
/// Lock managers are for ensuring only certain processes
/// get access to resources. They are abstracted away from
/// the interface of a process - all they need is some process
/// identifier. There are many ways of implementing this; see
/// two-phase-locking, wait-die, wound-wait.
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
	/// </summary>
	/// <param name="worker_id">The ID of the worker requesting the resource.</param>
	/// <param name="resource_id">The ID of the resource being requested.</param>
	/// <param name="lk_type">The type of lock being requested.</param>
	/// <returns>
	/// Success or failure. On failure, the worker is NOT added to a waiting
	/// list and should be aborted.
	/// </returns>
	virtual LockRequestResult request(size_t worker_id, size_t resource_id, LockType lk_type) = 0;

	/// <summary>
	/// This function is thread-safe.
	/// Determine if the lock manager is currently blocking
	/// the given worker. If so, you should NOT execute it!
	/// </summary>
	/// <param name="worker_id">The ID of the worker to check.</param>
	virtual void is_blocked(size_t worker_id) const = 0;

	/// <summary>
	/// This function is thread-safe.
	/// Call this when a worker finishes (and no longer needs
	/// any of its resources) or after being aborted (in which
	/// case all of its locks need to be released).
	/// Calling this function is rather crucial!
	/// </summary>
	/// <param name="worker_id">The ID of the worker to release all the locks of.</param>
	virtual void remove_worker(size_t worker_id) = 0;
};


} // namespace atpsearch


