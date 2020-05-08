/**
\file

\author Samuel Barrett

*/


#include "HMMConjectureTrainProcess.h"
#include "HMMModelLoaderProcess.h"
#include "HMMLoadTrainingDataProcess.h"
#include "HMMConjModelTrainProcess.h"
#include "HMMSaveModelParamsProcess.h"
#include "ProcessSequence.h"



namespace atp
{
namespace core
{


ATP_CORE_API ProcessPtr create_hmm_conjecture_train_process(
	db::DatabasePtr p_db, logic::LanguagePtr p_lang, size_t ctx_id,
	logic::ModelContextPtr p_ctx, size_t num_epochs,
	size_t max_dataset_size, boost::optional<size_t> model_id)
{
	auto p_proc = make_sequence<
		proc_data::HMMConjBuildingEssentials,
		proc_data::HMMConjModelEssentials,
		proc_data::HMMConjecturerTrainingEssentials,
		proc_data::HMMConjecturerTrainingEssentials>(
			boost::make_tuple(
				boost::bind(&create_hmm_model_loader_process, _1, _2),
				boost::bind(&create_hmm_training_data_load_proc,
					num_epochs, max_dataset_size, _1, _2),
				boost::bind(&create_conj_model_train_process, _1, _2),
				boost::bind(&create_save_hmm_model_params_proc, _1)
			));

	// set initial data block
	p_proc->get_data<0>().db = std::move(p_db);
	p_proc->get_data<0>().lang = std::move(p_lang);
	p_proc->get_data<0>().ctx = std::move(p_ctx);
	p_proc->get_data<0>().ctx_id = ctx_id;
	p_proc->get_data<0>().model_id = model_id;

	return p_proc;
}


}  // namespace core
}  // namespace atp


