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
	atp::logic::StatementArrayPtr p_target_stmts) :
	m_proc_state(ProcessState::AWAITING_LOCK),
	m_proof_state(ProofProcessState::INITIALISE_KERNEL),
	m_max_steps(search_settings.max_steps),
	m_step_size(search_settings.step_size),
	m_cur_step(0),
	m_lang(std::move(p_lang)),
	m_ctx(std::move(p_ctx)),
	m_db(std::move(p_db)),
	m_targets(std::move(p_target_stmts)),
	m_ker(m_lang->try_create_kernel(*m_ctx)),
	m_solver(search_settings.create_solver())
{
	m_ker->set_seed(search_settings.seed);
	m_solver->set_targets(m_targets);
	setup_init_kernel_operation();
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
		setup_save_results_operation();

		// output:

		// count the number successful / failed / unfinished:
		const auto states = m_solver->get_states();
		size_t num_true = 0, num_failed = 0,
			num_unfinished = 0;
		for (auto st : states)
			switch (st)
			{
			case atp::logic::ProofCompletionState::PROVEN:
				++num_true;
				break;
			case atp::logic::ProofCompletionState::NO_PROOF:
				++num_failed;
				break;
			case atp::logic::ProofCompletionState::UNFINISHED:
				++num_unfinished;
				break;
			}

		m_out << "Proof Process update --- ";
		m_out << "Done! Results:" << std::endl
			<< '\t' << num_true << " theorem(s) were proven true,"
			<< std::endl
			<< '\t' << num_failed << " theorem(s) have no proof,"
			<< std::endl
			<< '\t' << num_unfinished <<
			" theorem(s) did not finish in the allotted time."
			<< std::endl;

		m_out << "More details:" << std::endl;

		auto proofs = m_solver->get_proofs();
		auto times = m_solver->get_agg_time();
		auto mems = m_solver->get_max_mem();
		auto exps = m_solver->get_num_expansions();

		for (size_t i = 0; i < proofs.size(); i++)
		{
			switch (states[i])
			{
			case atp::logic::ProofCompletionState::PROVEN:
				m_out << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was successful; the statement is true."
					<< std::endl << "Proof:" << std::endl
					<< proofs[i]->to_str() << std::endl << std::endl;
				break;
			case atp::logic::ProofCompletionState::NO_PROOF:
				m_out << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was unsuccessful; it was impossible to prove "
					<< "using the given solver and the current settings."
					<< std::endl;
				break;
			case atp::logic::ProofCompletionState::UNFINISHED:
				m_out << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was unsuccessful; not enough time allocated."
					<< std::endl;
				break;
			}

			m_out << "Total time taken: " << times[i] << "s"
				<< std::endl;
			m_out << "Max nodes in memory: " << mems[i]
				<< std::endl;
			m_out << "Total node expansions: " << exps[i]
				<< std::endl;
			m_out << std::endl;
		}
	}
	else
	{
		m_solver->step(m_step_size);
		++m_cur_step;

		// output

		const auto states = m_solver->get_states();
		m_out << "Proof Process update --- ";
		m_out << (m_cur_step + 1) << '/'
			<< m_max_steps << " : " <<
			std::count(states.begin(), states.end(),
				atp::logic::ProofCompletionState::UNFINISHED) <<
			" proof(s) remaining.";
		m_out << std::endl;
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

		auto p_stmt_arr = atp::db::db_arr_to_stmt_arr(dbarr);

		m_ker->add_theorems(p_stmt_arr);

		m_out << "Proof Process update --- ";
		m_out << "Loaded " << p_stmt_arr->size() << " theorems from";
		m_out << " the theorem database!";
		m_out << std::endl;
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

		m_out << "Proof Process update --- ";
		m_out << "Finished saving the true theorems to the database";
		m_out << ", so they can be used in future proofs!";
		m_out << std::endl;
	}
	else
	{
		m_db_op->tick();
	}
}


