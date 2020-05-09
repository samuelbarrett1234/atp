/**
\file

\author Samuel Barrett

*/


#include "SaveConjResultsProcess.h"
#include "QueryProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


class ATP_CORE_API HMMSaveConjResultsProcess :
	public QueryProcess
{
public:
	HMMSaveConjResultsProcess(
		proc_data::HMMConjGenerationEssentials& gen_data) :
		QueryProcess(gen_data.db),
		m_gen_data(gen_data)
	{
		ATP_CORE_LOG(trace) << "Creating Save Proof Results "
			"process for HMM conjectures...";
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up result-saving query...";

		auto _p_bder = m_gen_data.db->create_query_builder(
			atp::db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

		auto p_bder = dynamic_cast<
			atp::db::ISaveProofResultsQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_context(m_gen_data.ctx_id, m_gen_data.ctx)
			->add_target_thms(m_gen_data.generated_stmts);

		return _p_bder;
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finishing saving proof results "
			"data for HMM conjectures.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to save proof results "
			"for HMM conjectures failed. Bailing...";
	}

private:
	proc_data::HMMConjGenerationEssentials& m_gen_data;
};


ProcessPtr create_save_conj_results_process(
	proc_data::HMMConjGenerationEssentials& gen_data)
{
	return std::make_shared<HMMSaveConjResultsProcess>(gen_data);
}


}  // namespace core
}  // namespace atp


