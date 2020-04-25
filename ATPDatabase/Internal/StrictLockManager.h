#pragma once


/**
\file

\author Samuel Barrett

\brief Contains one lock manager implementation.

*/


#include <map>
#include <mutex>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{


/**
\brief A lock manager which does not distinguish between read-only
	access and read-write access, thus is "stricter" as it is harder
	to obtain a read-only lock.
*/
class ATP_DATABASE_API StrictLockManager :
	public ILockManager
{
public:
	/**
	\brief The lock object corresponding to the strict lock manager.
	*/
	class ATP_DATABASE_API Lock :
		public ILock
	{
	public:
		Lock(StrictLockManager* p_parent,
			ResourceList my_resources);
		~Lock();

		inline ResourceList get_locked_resources() const
		{
			return m_my_resources;
		}

	private:
		StrictLockManager* const m_parent;
		ResourceList m_my_resources;
	};

public:
	LockPtr request_read_access(
		const ResourceList& res_list) override;
	LockPtr request_write_access(
		const ResourceList& res_list) override;
	LockPtr request_mixed_access(
		const ResourceList& res_list,
		const std::vector<bool>& writable) override;

private:
	std::mutex m_mutex;
	std::map<ResourceName, bool> m_locked;
};


}  // namespace db
}  // namespace atp


