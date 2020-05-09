/**
\file

\author Samuel Barrett

*/


#include "HMMConjModelTrainProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


class HMMConjModelTrainProc :
	public IProcess
{
public:
	HMMConjModelTrainProc(
		proc_data::HMMConjecturerTrainingEssentials& train) :
		m_train(train), m_cur_epoch(0)
	{
		ATP_CORE_LOG(info) << "Starting HMM conjecturer training...";

		if (m_train.num_epochs == 0)
			ATP_CORE_LOG(warning) << "Training process created, but "
			"zero epochs specified.";

		// do this once, and right at the start:
		m_train.model->estimate_free_q(m_train.dataset);
	}

	inline bool done() const override
	{
		return m_cur_epoch == m_train.num_epochs;
	}
	inline bool waiting() const override
	{
		return false;
	}
	inline bool has_failed() const override
	{
		return false;
	}

	void run_step() override
	{
		// train for N steps
		const size_t N = std::min(m_epochs_per_step,
			m_train.num_epochs - m_cur_epoch);

		m_train.model->train(
			m_train.dataset, N);
		m_cur_epoch += N;

		ATP_CORE_LOG(info) << "HMM conjecturer training: "
			<< m_cur_epoch << "/" << m_train.num_epochs <<
			" epochs completed.";
	}

private:
	static const size_t m_epochs_per_step = 10;
	size_t m_cur_epoch;
	proc_data::HMMConjecturerTrainingEssentials& m_train;
};


ATP_CORE_API ProcessPtr create_conj_model_train_process(
	proc_data::HMMConjecturerTrainingEssentials& train_in,
	proc_data::HMMConjecturerTrainingEssentials& train_out)
{
	// check inputs
	ATP_CORE_PRECOND(train_in.db != nullptr);
	ATP_CORE_PRECOND(train_in.lang != nullptr);
	ATP_CORE_PRECOND(train_in.ctx != nullptr);
	ATP_CORE_PRECOND(train_in.model != nullptr);

	// just directly move the data through:
	train_out = std::move(train_in);

	return std::make_shared<HMMConjModelTrainProc>(train_out);
}


}  // namespace core
}  // namespace atp


