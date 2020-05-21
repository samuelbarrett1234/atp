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


bool SimpleScheduler::update(std::list<ProcessPtr>& out_procs,
	size_t num_procs)
{
	ATP_CORE_LOG(info) << "Running scheduler...";

#ifdef _DEBUG
	static const size_t LOAD_FACTOR = 1;
	static const size_t NUM_THMS_PER_PROOF_PROC = 1;
#else
	static const size_t LOAD_FACTOR = 2;
	static const size_t NUM_THMS_PER_PROOF_PROC = 10;
#endif

	// cache this as it requires locking a mutex which is reasonably
	// expensive

	if (num_procs < LOAD_FACTOR * m_num_threads)
	{
		ATP_CORE_LOG(info) << "Adding "
			<< LOAD_FACTOR * m_num_threads - num_procs
			<< " new proof processes...";

		// create some extra processes:

		for (size_t i = 0; i <
			LOAD_FACTOR * m_num_threads - num_procs; ++i)
		{
			out_procs.push_back(create_rand_proof_process(m_db,
				NUM_THMS_PER_PROOF_PROC));
		}

		return true;
	}
	else return false;  // got enough already
}


}  // namespace core
}  // namespace atp


