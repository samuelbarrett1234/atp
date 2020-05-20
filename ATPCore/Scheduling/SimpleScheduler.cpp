/**
\file

\author Samuel Barrett

*/


#include "SimpleScheduler.h"
#include "../Processes/CreateHMMProcess.h"
#include "../Processes/ProofProcess.h"


namespace atp
{
namespace core
{


SimpleScheduler::SimpleScheduler(db::DatabasePtr p_db) :
	m_db(std::move(p_db)), m_num_threads(1)
{
	ATP_CORE_PRECOND(m_db != nullptr);
}


bool SimpleScheduler::update(ProcessManager& proc_mgr)
{
	static const size_t LOAD_FACTOR = 2;

	if (proc_mgr.num_procs_running() < LOAD_FACTOR * m_num_threads)
	{
		// create some extra processes:
		
		ATP_CORE_ASSERT(false && "not implemented yet!");
	}
	else return false;  // got enough already
}


}  // namespace core
}  // namespace atp


