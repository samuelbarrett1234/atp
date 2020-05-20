/**
\file

\author Samuel Barrett

*/


#include <boost/bind.hpp>
#include "ProofProcess.h"
#include "CommonProcessData.h"
#include "ProcessSequence.h"
#include "UnprovenThmSelectorProcess.h"
#include "ProofInitProcess.h"
#include "RunSolverProcess.h"
#include "SaveProofResultsProcess.h"
#include "SelectContextProcess.h"
#include "SelectSearchSettingsProcess.h"


namespace atp
{
namespace core
{


ProcessPtr create_proof_process(
	logic::LanguagePtr p_lang, size_t ctx_id, size_t ss_id,
	logic::ModelContextPtr p_ctx, db::DatabasePtr p_db,
	search::SearchSettings& search_settings,
	logic::StatementArrayPtr p_target_stmts)
{
	auto p_proc = make_sequence<
		proc_data::ProofSetupEssentials,
		proc_data::ProofEssentials,
		proc_data::ProofEssentials>(
			boost::make_tuple(
		boost::bind(&create_proof_init_process, _1, _2),
		boost::bind(&create_run_solver_process, _1, _2),
		boost::bind(&create_save_results_process, _1)
		));

	// set initial data block
	p_proc->get_data<0>().db = std::move(p_db);
	p_proc->get_data<0>().lang = std::move(p_lang);
	p_proc->get_data<0>().ctx = std::move(p_ctx);
	p_proc->get_data<0>().ctx_id = ctx_id;
	p_proc->get_data<0>().ss_id = ss_id;
	p_proc->get_data<0>().settings = search_settings;
	p_proc->get_data<0>().target_thms =  // normalise the statements!
		p_proc->get_data<0>().lang->normalise(p_target_stmts);

	ATP_CORE_LOG(debug) << "Created proof process to handle " <<
		p_target_stmts->size() << " target statements, but there "
		"were " << p_proc->get_data<0>().target_thms->size() <<
		" after normalisation.";

	return p_proc;
}


ProcessPtr create_proof_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db,
	search::SearchSettings& search_settings,
	size_t num_targets)
{
	auto p_proc = make_sequence<
		proc_data::ProofSetupEssentials,
		proc_data::ProofSetupEssentials,
		proc_data::ProofEssentials,
		proc_data::ProofEssentials>(
			boost::make_tuple(
				boost::bind(&create_unproven_thm_select_proc,
					num_targets, _1, _2),
				boost::bind(&create_proof_init_process, _1, _2),
				boost::bind(&create_run_solver_process, _1, _2),
				boost::bind(&create_save_results_process, _1)
			));

	// set initial data block
	p_proc->get_data<0>().db = std::move(p_db);
	p_proc->get_data<0>().lang = std::move(p_lang);
	p_proc->get_data<0>().ctx = std::move(p_ctx);
	p_proc->get_data<0>().ctx_id = ctx_id;
	p_proc->get_data<0>().ss_id = ss_id;
	p_proc->get_data<0>().settings = search_settings;
	// don't set target_thms

	return p_proc;
}


ProcessPtr create_rand_proof_process(
	db::DatabasePtr p_db, size_t num_to_prove)
{
	auto p_proc = make_sequence<
		proc_data::DatabaseEssentials,
		proc_data::LogicEssentials,
		proc_data::ProofSetupEssentials,
		proc_data::ProofSetupEssentials,
		proc_data::ProofEssentials,
		proc_data::ProofEssentials>(
			boost::make_tuple(
				boost::bind(&create_select_ctx_process, _1, _2),
				boost::bind(&create_select_ss_process, _1, _2),
				boost::bind(&create_unproven_thm_select_proc,
					num_to_prove, _1, _2),
				boost::bind(&create_proof_init_process, _1, _2),
				boost::bind(&create_run_solver_process, _1, _2),
				boost::bind(&create_save_results_process, _1)
			));

	// set initial data
	p_proc->get_data<0>().db = std::move(p_db);

	return p_proc;
}


}  // namespace core
}  // namespace atp


