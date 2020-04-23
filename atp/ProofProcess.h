#pragma once


/**
\file

\author Samuel Barrett

\brief A process for proving a set of statements.

*/


#include <sstream>
#include "IProcess.h"
#include <ATPLogic.h>
#include <ATPSearch.h>
#include <ATPDatabase.h>


class ProofProcess :
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
	*/
	ProofProcess(atp::logic::LanguagePtr p_lang,
		atp::logic::ModelContextPtr p_ctx,
		atp::db::DatabasePtr p_db,
		atp::search::SearchSettings& search_settings,
		atp::logic::StatementArrayPtr p_target_stmts);

	inline ProcessState state() const override
	{
		return m_proc_state;
	}
	void run_step() override;
	void try_acquire_lock() override;
	inline void dump_log(std::ostream& out) override
	{
		out << m_out.str();
		m_out.clear();
	}

private:
	void step_solver();
	void init_kernel();
	void save_results();
	void setup_init_kernel_operation();
	void setup_save_results_operation();

private:
	std::stringstream m_out;
	ProcessState m_proc_state;
	ProofProcessState m_proof_state;
	const size_t m_max_steps, m_step_size;
	size_t m_cur_step;
	atp::logic::LanguagePtr m_lang;
	atp::logic::ModelContextPtr m_ctx;
	atp::db::DatabasePtr m_db;
	atp::logic::KnowledgeKernelPtr m_ker;
	atp::search::SolverPtr m_solver;
	atp::logic::StatementArrayPtr m_targets;

	// these are either for init kernel or save results
	atp::db::OpStarterPtr m_op_starter;
	atp::db::DBOpPtr m_db_op;
};


