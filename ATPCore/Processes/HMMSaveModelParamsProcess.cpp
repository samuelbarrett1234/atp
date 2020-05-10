/**
\file

\author Samuel Barrett

*/


#include "HMMSaveModelParamsProcess.h"
#include "../Models/HMMConjectureModel.h"
#include "QueryProcess.h"


namespace atp
{
namespace core
{


class HMMSaveModelParamsProc :
	public QueryProcess
{
public:
	HMMSaveModelParamsProc(
		proc_data::HMMConjecturerTrainingEssentials& train_data) :
		m_train_data(train_data),
		QueryProcess(train_data.db)
	{
		ATP_CORE_LOG(info) << "Saving HMM model parameters...";
	}

protected:
	virtual db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Creating query for saving HMM model "
			"parameters to the database.";

		auto _p_bder = m_train_data.db->create_query_builder(
			db::QueryBuilderType::SAVE_HMM_CONJ_PARAMS);

		auto p_bder = dynamic_cast<db::ISaveHmmConjModelParams*>(
			_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		const size_t N = m_train_data.model->num_states();

		p_bder->set_ctx(m_train_data.ctx_id, m_train_data.ctx)
			->set_free_q(m_train_data.model->free_q())
			->set_num_states(N)
			->set_model_id(m_train_data.model_id);

		const auto symbs = m_train_data.model->get_symbols();
		for (size_t i = 0; i < N; ++i)
		{
			for (size_t j = 0; j < N; ++j)
			{
				p_bder->add_state_trans(i, j,
					m_train_data.model->st_trans_prob(i, j));
			}

			for (size_t id : symbs)
			{
				p_bder->add_observation(i, id,
					m_train_data.model->obs_prob(i, id));
			}
		}

		return _p_bder;
	}

private:
	proc_data::HMMConjecturerTrainingEssentials& m_train_data;
};


ProcessPtr create_save_hmm_model_params_proc(
	proc_data::HMMConjecturerTrainingEssentials& train_data)
{
	// check inputs
	ATP_CORE_PRECOND(train_data.db != nullptr);
	ATP_CORE_PRECOND(train_data.lang != nullptr);
	ATP_CORE_PRECOND(train_data.ctx != nullptr);
	ATP_CORE_PRECOND(train_data.model != nullptr);

	return std::make_shared<HMMSaveModelParamsProc>(train_data);
}


}  // namespace core
}  // namespace atp


