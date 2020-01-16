#include "LockManager.h"


// Author: Samuel Barrett


namespace atpsearch
{


FreezeLock::FreezeLock(ILockManager& lkmgr) :
	worker_id(0),  // could be anything
	lkmgr(lkmgr),
	bIsALock(false)
{ }


FreezeLock::FreezeLock(size_t worker_id, ILockManager& lkmgr) :
	worker_id(worker_id),
	lkmgr(lkmgr),
	bIsALock(true)
{ }


FreezeLock::~FreezeLock()
{
	if (bIsALock)
	{
		lkmgr.unfreeze_worker(worker_id);
	}
}


} // namespace atpsearch


