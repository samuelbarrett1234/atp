/**
\file

\author Samuel Barrett

*/


#include "ProofProcess.h"
#include "ATP.h"


ProofProcess::ProofProcess(
	atp::logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	atp::logic::ModelContextPtr p_ctx,
	atp::db::DatabasePtr p_db,
	atp::search::SearchSettings& search_settings,
	atp::logic::StatementArrayPtr p_target_stmts) :
	m_done(false),
	m_proof_state(ProofProcessState::INITIALISE_KERNEL),
	m_max_steps(search_settings.max_steps),
	m_step_size(search_settings.step_size),
	m_cur_step(0),
	m_lang(std::move(p_lang)),
	m_ctx(std::move(p_ctx)),
	m_db(std::move(p_db)),
	m_targets(std::move(p_target_stmts)),
	m_ker(m_lang->try_create_kernel(*m_ctx)),
	m_solver(search_settings.create_solver(m_ker)),
	m_ctx_id(ctx_id), m_ss_id(ss_id)
{
	ATP_PRECOND(m_solver != nullptr);  // will fail if bad model ctx
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


void ProofProcess::step_solver()
{
	if (!m_solver->any_proof_not_done() ||
		m_cur_step == m_max_steps)
	{
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
		m_out << m_cur_step << '/'
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

	switch (m_db_op->state())
	{
	case atp::db::TransactionState::COMPLETED:
	{
		// our next state will be to run the proof
		m_proof_state = ProofProcessState::RUNNING_PROOF;
		
		// set the statements in the kernel:
		m_helpers = m_lang->deserialise_stmts(m_temp_results,
			atp::logic::StmtFormat::TEXT, *m_ctx);

		if (m_helpers == nullptr)
		{
			m_out << "ERROR! Failed to initialise kernel. There was"
				<< " a problem with the following batch of "
				<< "statements retrieved from the database: " << '"'
				<< m_temp_results.str() << '"' << std::endl;
		}
		else
		{
			m_out << "Proof Process update --- ";
			m_out << "Loaded " << m_helpers->size() << " theorems";
			m_out << " from the theorem database!" << std::endl;

			m_ker->add_theorems(m_helpers);
		}

		m_temp_results = std::stringstream();  // reset this
		m_db_op.reset();  // and this
	}
		break;

	case atp::db::TransactionState::RUNNING:
	{
		// still running the query; extract the current results:

		auto p_readable_op = dynamic_cast<atp::db::IQueryTransaction*>
			(m_db_op.get());

		ATP_ASSERT(p_readable_op != nullptr);

		if (p_readable_op->has_values())
		{
			if (p_readable_op->arity() != 1)
			{
				m_out << "ERROR! Failed to initialise kernel. Kernel"
					<< " initialisation query returned an arity of "
					<< p_readable_op->arity() << ", which differed "
					<< "from the expected result of 1. Ignoring "
					<< "query..." << std::endl;
			}
			else
			{
				atp::db::DValue stmt_str;

				if (!p_readable_op->try_get(0, atp::db::DType::STR,
					&stmt_str))
				{
					m_out << "WARNING! Encountered non-string "
						<< "statement value in database. Type "
						<< "could be null?" << std::endl;
				}
				else
				{
					m_temp_results << boost::variant2::get<2>(
						stmt_str) << std::endl;
				}
			}
		}

		m_db_op->step();
	}
		break;

	default:
		m_out << "ERROR! Failed to initialise kernel. Database query"
			<< " failed unexpectedly. Ignoring query and proceeding"
			<< "..." << std::endl;
		m_proof_state = ProofProcessState::RUNNING_PROOF;
		m_temp_results = std::stringstream();  // reset this
		m_db_op.reset();  // and this
		break;
	}
}


void ProofProcess::save_results()
{
	ATP_ASSERT(m_db_op != nullptr);

	switch (m_db_op->state())
	{
	case atp::db::TransactionState::RUNNING:
		m_db_op->step();
		break;

	case atp::db::TransactionState::COMPLETED:
		m_done = true;
		m_db_op.reset();
		m_out << "Proof Process update --- ";
		m_out << "Finished saving the true theorems to the database";
		m_out << ", so they can be used in future proofs!";
		m_out << std::endl;
		break;

	default:
		m_out << "ERROR! Failed to save proof results. Unexpected "
			<< "error while executing query." << std::endl;
		m_done = true;
		m_db_op.reset();
		break;
	}
}


void ProofProcess::setup_init_kernel_operation()
{
	ATP_ASSERT(m_db_op == nullptr);

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::RANDOM_PROVEN_THM_SELECTION);

	auto p_bder = dynamic_cast<
		atp::db::IRndProvenThmSelectQryBder*>(_p_bder.get());

	ATP_ASSERT(p_bder != nullptr);

	p_bder->set_limit(25  /* todo: don't hardcode */)
		->set_context(m_ctx_id, m_ctx);

	const auto query = p_bder->build();

	m_db_op = m_db->begin_transaction(query);

	ATP_ASSERT(m_db_op != nullptr);
}


void ProofProcess::setup_save_results_operation()
{
	ATP_ASSERT(m_db_op == nullptr);

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

	auto p_bder = dynamic_cast<
		atp::db::ISaveProofResultsQryBder*>(_p_bder.get());

	ATP_ASSERT(p_bder != nullptr);

	p_bder->set_context(m_ctx_id, m_ctx)
		->set_search_settings(m_ss_id)
		->add_proof_states(m_solver->get_proofs())
		->add_target_thms(m_targets)
		->add_helper_thms(m_helpers)
		->add_proof_times(m_solver->get_agg_time())
		->add_max_mem_usages(m_solver->get_max_mem())
		->add_num_node_expansions(m_solver->get_num_expansions());

	const auto query = p_bder->build();

	m_db_op = m_db->begin_transaction(query);

	ATP_ASSERT(m_db_op != nullptr);
}


