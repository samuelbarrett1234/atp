#pragma once


/**
\file

\author Samuel Barrett

\brief A process for proving a set of statements.

*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPSearch.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


ATP_CORE_API ProcessPtr make_proof_process(
	atp::logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	atp::logic::ModelContextPtr p_ctx,
	atp::db::DatabasePtr p_db,
	atp::search::SearchSettings& search_settings,
	atp::logic::StatementArrayPtr p_target_stmts);


class ATP_CORE_API ProofProcess :
	public IProcess
{
private:
	// different states we go through while proving
	enum class ProofProcessState
	{
		// we are filling the kernel with already-proven theorems
		// (which requires DB access)
		INITIALISE_KERNEL,
		// we are performing the search operation
		RUNNING_PROOF,
		// we are saving the results into the database
		SAVE_RESULTS
	};

public:
	/**
	\param finish_callback A function to call when the solver is
		finished.

	\param ctx_id The ID of the given model context from the data-
		base. This cannot be obtained via a lookup from p_ctx or
		p_db.
	*/
	ProofProcess(atp::logic::LanguagePtr p_lang,
		size_t ctx_id, size_t ss_id,
		atp::logic::ModelContextPtr p_ctx,
		atp::db::DatabasePtr p_db,
		atp::search::SearchSettings& search_settings,
		atp::logic::StatementArrayPtr p_target_stmts);

	inline bool done() const override
	{
		return m_done;
	}
	inline bool waiting() const override
	{
		return (m_db_op != nullptr && m_db_op->state() ==
			atp::db::TransactionState::RUNNING &&
			m_db_op->waiting());
	}
	void run_step() override;

private:
	void step_solver();
	void init_kernel();
	void save_results();
	void setup_init_kernel_operation();
	void setup_save_results_operation();

private:
	bool m_done;
	ProofProcessState m_proof_state;
	const size_t m_max_steps, m_step_size;
	size_t m_cur_step;
	const size_t m_ctx_id;  // from DB
	const size_t m_ss_id;  // from DB
	atp::logic::LanguagePtr m_lang;
	atp::logic::ModelContextPtr m_ctx;
	atp::db::DatabasePtr m_db;
	atp::logic::KnowledgeKernelPtr m_ker;
	atp::search::SolverPtr m_solver;
	atp::logic::StatementArrayPtr m_targets, m_helpers;

	// these are either for init kernel or save results
	atp::db::TransactionPtr m_db_op;

	// this is for storing intermediate results:
	std::stringstream m_temp_results;
};


}  // namespace core
}  // namespace atp


