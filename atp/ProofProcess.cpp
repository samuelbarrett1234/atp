/**
\file

\author Samuel Barrett

*/


#include "ProofProcess.h"
#include "ATP.h"


ProofProcess::ProofProcess(
	atp::logic::LanguagePtr p_lang,
	atp::logic::ModelContextPtr p_ctx,
	atp::db::DatabasePtr p_db,
	atp::search::SearchSettings& search_settings,
	atp::logic::StatementArrayPtr p_target_stmts,
	std::function<void(atp::search::SolverPtr)> finish_callback) :
	m_proc_state(ProcessState::AWAITING_LOCK),
	m_proof_state(ProofProcessState::INITIALISE_KERNEL),
	m_max_steps(search_settings.max_steps),
	m_step_size(search_settings.step_size),
	m_cur_step(0),
	m_lang(std::move(p_lang)),
	m_ctx(std::move(p_ctx)),
	m_db(std::move(p_db)),
	m_ker(m_lang->try_create_kernel(*m_ctx)),
	on_finished(finish_callback),
	m_solver(search_settings.create_solver())
{
	m_ker->set_seed(search_settings.seed);
	m_solver->set_targets(std::move(p_target_stmts));

	// TODO: set up kernel-initialising database operation
	ATP_ASSERT(false);
}


void ProofProcess::run_step()
{
	switch (m_proof_state)
	{
	case ProofProcessState::INITIALISE_KERNEL:
		init_kernel();
		break;
	case ProofProcessState::RUNNING_PROOF:
		step_solver();
		break;
	case ProofProcessState::SAVE_RESULTS:
		save_results();
		break;
	}
}


void ProofProcess::try_acquire_lock()
{
	ATP_ASSERT(m_op_starter != nullptr);
	ATP_ASSERT(m_db_op == nullptr);

	/**
	\todo: be more careful about whether we need read/write access.
	*/

	// try to obtain a lock
	auto p_lock = m_db->lock_mgr().request_write_access(
		m_op_starter->res_needed());

	if (p_lock != nullptr)
	{
		// got it!
		m_db_op = m_op_starter->lock_obtained(std::move(p_lock));
		m_proc_state = ProcessState::RUNNING;
		m_op_starter.reset();
	}
	// else do nothing and try again next time
}


void ProofProcess::step_solver()
{
	if (!m_solver->any_proof_not_done() ||
		m_cur_step == m_max_steps)
	{
		m_proc_state = ProcessState::AWAITING_LOCK;
		m_proof_state = ProofProcessState::SAVE_RESULTS;
		on_finished(m_solver);
	}
	else
	{
		m_solver->step(m_step_size);
		++m_cur_step;
	}
}


void ProofProcess::init_kernel()
{
	ATP_ASSERT(m_db_op != nullptr);
	ATP_ASSERT(m_op_starter == nullptr);

	if (m_db_op->done())
	{
		// get results of call
		// set them as a statement array
		auto p_readable_op = dynamic_cast<atp::db::IDBReadOperation*>
			(m_db_op.get());

		ATP_ASSERT(p_readable_op != nullptr);
		ATP_ASSERT(p_readable_op->num_cols() == 1);

		auto dbarr = p_readable_op->col_values(0);

		auto p_stmt_arr = atp::db::db_to_stmt_arr(dbarr);

		m_ker->add_theorems(p_stmt_arr);
	}
	else
	{
		m_db_op->tick();
	}
}


void ProofProcess::save_results()
{
	ATP_ASSERT(m_db_op != nullptr);
	ATP_ASSERT(m_op_starter == nullptr);

	if (m_db_op->done())
	{
		m_db_op.reset();

		m_proc_state = ProcessState::DONE;
	}
	else
	{
		m_db_op->tick();
	}
}


