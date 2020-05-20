#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a very elementary scheduler algorithm
*/


#include <ATPDatabase.h>
#include "IScheduler.h"


namespace atp
{
namespace core
{


class ATP_CORE_API SimpleScheduler :
	public IScheduler
{
public:
	SimpleScheduler(db::DatabasePtr p_db);

	bool update(ProcessManager& proc_mgr) override;
	inline void set_num_threads(size_t nt) override
	{
		ATP_CORE_PRECOND(nt > 0);
		m_num_threads = nt;
	}

private:
	db::DatabasePtr m_db;
	size_t m_num_threads;
};


}  // namespace core
}  // namespace atp


