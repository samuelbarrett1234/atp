#pragma once


/**
\file

\author Samuel Barrett

\brief Contains code to do with locking and resource access control.
*/


#include <memory>
#include "../ATPDatabaseAPI.h"
#include "Data.h"


namespace atp
{
namespace db
{


class ILock;  // forward declaration
typedef std::shared_ptr<ILock> LockPtr;


/**
\brief This object dishes out locks as entities require them.

\details Some lock managers may wish to distinguish between read and
	write locks, however that is not always necessary.

\warning This class is thread-safe, however you may run into issues
	if you try to request several locks for one operation. In order
	to prevent deadlocks, it is very important that you calculate all
	of the resources you will need BEFORE GETTING THE LOCK, and then
	to request the lock all at once.
*/
class ATP_DATABASE_API ILockManager
{
public:
	virtual ~ILockManager() = default;

	/**
	\brief Ask for read-only access to the given set of resources.

	\returns A lock object if and only if the lock was granted. The
		lock is released as soon as this lock object is destroyed,
		and then access is no longer granted.

	\warning Of course, it is crucial to (i) check the return result
		of this function, and (ii) keep the returned object around
		for as long as you need the lock, but preferably no longer.
	*/
	virtual LockPtr request_read_access(
		const ResourceList& res_list) = 0;

	/**
	\brief Ask for read-write access to the given set of resources.

	\returns A lock object if and only if the lock was granted. The
		lock is released as soon as this lock object is destroyed,
		and then access is no longer granted.

	\warning Of course, it is crucial to (i) check the return result
		of this function, and (ii) keep the returned object around
		for as long as you need the lock, but preferably no longer.
	*/
	virtual LockPtr request_write_access(
		const ResourceList& res_list) = 0;

	/**
	\brief For a given set of resources, ask for read-write access on
		some, and read-only access on others.

	\param writable An array which corresponds to `res_list` and
		writable[i] is true iff we want write-access to res_list[i].

	\pre res_list.size() == writable.size()

	\returns A lock object if and only if the lock was granted. The
		lock is released as soon as this lock object is destroyed,
		and then access is no longer granted.

	\warning Of course, it is crucial to (i) check the return result
		of this function, and (ii) keep the returned object around
		for as long as you need the lock, but preferably no longer.
	*/
	virtual LockPtr request_mixed_access(
		const ResourceList& res_list,
		const std::vector<bool>& writable) = 0;
};


/**
\brief This object represents an active lock on a set of reources.

\note The lock is released only when this object is destroyed.
*/
class ATP_DATABASE_API ILock
{
public:
	virtual ~ILock() = default;
	
	virtual ResourceList get_locked_resources() const = 0;
};


}  // namespace db
}  // namespace atp


