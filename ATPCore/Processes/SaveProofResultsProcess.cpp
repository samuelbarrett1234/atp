/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SaveProofResultsProcess.h"
#include "QueryProcess.h"


namespace atp
{
namespace core
{


class SaveProofResultsProcess :
	public QueryProcess
{
public:
	SaveProofResultsProcess(
		proc_data::ProofEssentials& proof_data) :
		QueryProcess(proof_data.db),
		m_pf_data(proof_data)
	{
		ATP_CORE_LOG(trace) << "Creating Save Proof Results "
			"process...";
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up result-saving query...";

		auto _p_bder = m_pf_data.db->create_query_builder(
			atp::db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

		auto p_bder = dynamic_cast<
			atp::db::ISaveProofResultsQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_context(m_pf_data.ctx_id, m_pf_data.ctx)
			->set_search_settings(m_pf_data.ss_id)
			->add_proof_states(m_pf_data.solver->get_proofs())
			->add_target_thms(m_pf_data.target_thms)
			->add_helper_thms(m_pf_data.helper_thms)
			->add_proof_times(m_pf_data.solver->get_agg_time())
			->add_max_mem_usages(m_pf_data.solver->get_max_mem())
			->add_num_node_expansions(
				m_pf_data.solver->get_num_expansions());

		return _p_bder;
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finishing saving proof results "
			"data.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to save proof results "
			"failed. Bailing...";
	}

private:
	proc_data::ProofEssentials& m_pf_data;
};


ProcessPtr create_save_results_process(
	proc_data::ProofEssentials& proof_data_before,
	proc_data::ProofEssentials& proof_data_after)
{
	// check all the data is valid:
	ATP_CORE_PRECOND(proof_data_before.db != nullptr);
	ATP_CORE_PRECOND(proof_data_before.lang != nullptr);
	ATP_CORE_PRECOND(proof_data_before.ctx != nullptr);
	ATP_CORE_PRECOND(proof_data_before.target_thms != nullptr);
	ATP_CORE_PRECOND(proof_data_before.ker != nullptr);
	ATP_CORE_PRECOND(proof_data_before.solver != nullptr);
	ATP_CORE_PRECOND(proof_data_before.helper_thms != nullptr);
	ATP_CORE_PRECOND(proof_data_before.max_steps > 0);
	ATP_CORE_PRECOND(proof_data_before.step_size > 0);

	// copy data over, it stays the same
	if (&proof_data_before != &proof_data_after)
		proof_data_after = proof_data_before;

	return std::make_shared<SaveProofResultsProcess>(
		proof_data_after);
}


}  // namespace core
}  // namespace atp


