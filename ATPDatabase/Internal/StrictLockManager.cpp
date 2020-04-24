/**
\file

\author Samuel Barrett

*/


#include "StrictLockManager.h"


namespace atp
{
namespace db
{


LockPtr StrictLockManager::request_read_access(
	const ResourceList& res_list)
{
	// strictness: treat everything as a read/write lock
	return request_write_access(res_list);
}


LockPtr StrictLockManager::request_write_access(
	const ResourceList& res_list)
{
	std::scoped_lock<std::mutex> mut_lock(m_mutex);

	typedef std::map<ResourceName, bool>::iterator MapIter;

	std::vector<MapIter> res_iters;
	res_iters.reserve(res_list.size());
	for (auto res : res_list)
	{
		auto iter = m_locked.find(res);

		// if this resource is already locked...
		if (iter != m_locked.end() && iter->second)
			return LockPtr();

		res_iters.push_back(iter);
	}

	// if we get to this point, none of the resources were locked and
	// we may proceed
	for (MapIter& iter : res_iters)
	{
		if (iter == m_locked.end())
		{
			// not a resource we have seen before
			m_locked[iter->first] = true;
		}
		else
		{
			iter->second = true;
		}
	}

	return std::make_shared<Lock>(this, res_list);
}


LockPtr StrictLockManager::request_mixed_access(
	const ResourceList& res_list, const std::vector<bool>& writable)
{
	// strictness: treat everything as a read/write lock
	return request_write_access(res_list);
}


StrictLockManager::Lock::Lock(
	StrictLockManager* p_parent, ResourceList my_resources) :
	m_parent(p_parent), m_my_resources(std::move(my_resources))
{ }


StrictLockManager::Lock::~Lock()
{
	std::scoped_lock<std::mutex> mut_lock(m_parent->m_mutex);

	for (auto res : m_my_resources)
	{
		auto iter = m_parent->m_locked.find(res);

		ATP_DATABASE_ASSERT(iter != m_parent->m_locked.end());
		ATP_DATABASE_ASSERT(iter->second);

		iter->second = false;  // now unlocked
	}
}


}  // namespace db
}  // namespace atp


