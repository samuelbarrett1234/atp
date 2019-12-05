#pragma once



// Author: Samuel Barrett


#include <typeinfo>
#include "Locks.h"


namespace atp
{


class IProcess;


/// <summary>
/// This class is passed to a specific process, and acquires
/// locks for that specific process.
/// This class exists for two reasons. Firstly, it means we
/// don't need to give processes access to the full lock manager
/// interface in order to request locks. Secondly, it means
/// the process can only acquire locks for itself (which we
/// obviously want to enforce) whereas using the lock manager
/// interface it would be possible to acquire locks while appearing
/// like another process.
/// </summary>
class LockBroker
{
public:
	LockBroker(LockManager& mgr, const IProcess* pProc) :
		m_mgr(mgr),
		m_pProc(pProc)
	{ }

	/// <summary>
	/// Acquire an exclusive (read/write) lock on the given
	/// resource type.
	/// </summary>
	template<typename Val_t>
	XLock<Val_t> acquire_x()
	{
		return XLock<Val_t>(acquire_x(typeid(Val_t).hash_code()));
	}

	/// <summary>
	/// Acquire a shared (read only) lock on the given
	/// resource type.
	/// </summary>
	template<typename Val_t>
	SLock<Val_t> acquire_s()
	{
		return SLock<Val_t>(acquire_s(typeid(Val_t).hash_code()));
	}

private:
	XLockGeneric acquire_x(size_t type_hash_code);
	SLockGeneric acquire_s(size_t type_hash_code);

private:
	LockManager& m_mgr;
	const IProcess* m_pProc;
};


/// <summary>
/// This class allows processes to request locks for various
/// resources, and tracks what locks have been acquired by each
/// process.
/// </summary>
class LockManager
{
public:
	LockBroker get_broker(const IProcess* pProc);

	bool is_waiting(const IProcess* pProc) const;
};


} // namespace atp


