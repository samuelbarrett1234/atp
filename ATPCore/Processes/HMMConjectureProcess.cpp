/**
\file

\author Samuel Barrett

*/


#include "HMMConjectureProcess.h"
#include "HMMModelLoaderProcess.h"
#include "HMMGenerateConjProcess.h"
#include "SaveConjResultsProcess.h"
#include "ProcessSequence.h"


namespace atp
{
namespace core
{


ProcessPtr create_hmm_conjecture_process(
	db::DatabasePtr p_db,
	logic::LanguagePtr p_lang, size_t ctx_id,
	logic::ModelContextPtr p_ctx, size_t num_to_generate,
	boost::optional<size_t> model_id)
{
	auto p_proc = make_sequence<
		proc_data::HMMConjBuildingEssentials,
		proc_data::HMMConjModelEssentials,
		proc_data::HMMConjGenerationEssentials>(
			boost::make_tuple(
				boost::bind(&create_hmm_model_loader_process, _1, _2),
				boost::bind(&create_hmm_conj_gen_process,
					num_to_generate, _1, _2),
				boost::bind(&create_save_conj_results_process, _1)
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


