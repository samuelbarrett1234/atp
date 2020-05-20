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
	static const size_t NUM_THMS_PER_PROOF_PROC = 10;

	// cache this as it requires a mutex lock which is reasonably
	// expensive
	const size_t num_procs = proc_mgr.num_procs_running();

	if (num_procs < LOAD_FACTOR * m_num_threads)
	{
		// create some extra processes:

		for (size_t i = 0; i <
			LOAD_FACTOR * m_num_threads - num_procs; ++i)
		{
			proc_mgr.add(create_rand_proof_process(m_db,
				NUM_THMS_PER_PROOF_PROC));
		}

		return true;
	}
	else return false;  // got enough already
}


}  // namespace core
}  // namespace atp


