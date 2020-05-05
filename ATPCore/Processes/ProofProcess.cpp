/**
\file

\author Samuel Barrett

*/


#include "ProofProcess.h"


namespace atp
{
namespace core
{


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
	ATP_CORE_LOG(trace) << "Created a new proof process.";
	ATP_CORE_PRECOND(m_solver != nullptr);  // will fail if bad model ctx
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
		ATP_CORE_LOG(trace) << "Finished running solver, for "
			<< m_cur_step << "/" << m_max_steps << " steps.";

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

		ATP_CORE_LOG(info) << "Proof Process update --- "
			<< "Proof process finished proving! Results:" << std::endl
			<< '\t' << num_true << " theorem(s) were proven true,"
			<< std::endl
			<< '\t' << num_failed << " theorem(s) have no proof,"
			<< std::endl
			<< '\t' << num_unfinished <<
			" theorem(s) did not finish in the allotted time."
			<< std::endl
			<< "More details:" << std::endl;

		auto proofs = m_solver->get_proofs();
		auto times = m_solver->get_agg_time();
		auto mems = m_solver->get_max_mem();
		auto exps = m_solver->get_num_expansions();

		for (size_t i = 0; i < proofs.size(); i++)
		{
			switch (states[i])
			{
			case atp::logic::ProofCompletionState::PROVEN:
				ATP_CORE_LOG(info) << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was successful; the statement is true.";
				break;
			case atp::logic::ProofCompletionState::NO_PROOF:
				ATP_CORE_LOG(info) << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was unsuccessful; it was impossible to prove "
					<< "using the given solver and the current settings.";
				break;
			case atp::logic::ProofCompletionState::UNFINISHED:
				ATP_CORE_LOG(info) << "Proof of \"" << m_targets->at(i).to_str()
					<< "\" was unsuccessful; not enough time allocated.";
				break;
			}

			ATP_CORE_LOG(info) << "Total time taken: " << times[i] << "s";
			ATP_CORE_LOG(info) << "Max nodes in memory: " << mems[i];
			ATP_CORE_LOG(info) << "Total node expansions: " << exps[i];
		}
	}
	else
	{
		m_solver->step(m_step_size);
		++m_cur_step;

		// output

		const auto states = m_solver->get_states();
		ATP_CORE_LOG(info) << "Proof Process update --- "
			<< "Step " << m_cur_step << '/'
			<< m_max_steps << " : " <<
			std::count(states.begin(), states.end(),
				atp::logic::ProofCompletionState::UNFINISHED) <<
			" proof(s) remaining.";
	}
}


void ProofProcess::init_kernel()
{
	ATP_CORE_ASSERT(m_db_op != nullptr);

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
			ATP_CORE_LOG(error) << "Failed to initialise kernel. There was"
				<< " a problem with the following batch of "
				<< "statements retrieved from the database: " << '"'
				<< m_temp_results.str() << '"';
		}
		else if (m_helpers->size() > 0)
		{
			ATP_CORE_LOG(info) << "Proof Process update --- "
				<< "Loaded " << m_helpers->size() << " theorems"
				<< " from the theorem database!";

			m_ker->add_theorems(m_helpers);
		}
		else ATP_CORE_LOG(warning) << "Proof process could not find any "
			"theorems to load from the database.";

		m_temp_results = std::stringstream();  // reset this
		m_db_op.reset();  // and this
	}
	break;

	case atp::db::TransactionState::RUNNING:
	{
		// still running the query; extract the current results:

		auto p_readable_op = dynamic_cast<atp::db::IQueryTransaction*>
			(m_db_op.get());

		ATP_CORE_ASSERT(p_readable_op != nullptr);

		if (p_readable_op->has_values())
		{
			if (p_readable_op->arity() != 1)
			{
				ATP_CORE_LOG(error) << "Failed to initialise kernel. Kernel"
					<< " initialisation query returned an arity of "
					<< p_readable_op->arity() << ", which differed "
					<< "from the expected result of 1. Ignoring "
					<< "query...";
			}
			else
			{
				atp::db::DValue stmt_str;

				if (!p_readable_op->try_get(0, atp::db::DType::STR,
					&stmt_str))
				{
					ATP_CORE_LOG(warning) << "Encountered non-string "
						<< "statement value in database. Type "
						<< "could be null?";
				}
				else
				{
					ATP_CORE_LOG(debug) << "Adding '" << atp::db::get_str(stmt_str)
						<< "' to helper theorems.";

					m_temp_results << atp::db::get_str(stmt_str) << std::endl;
				}
			}
		}

		m_db_op->step();
	}
	break;

	default:
		ATP_CORE_LOG(error) << "Failed to initialise kernel. Database query"
			<< " failed unexpectedly. Ignoring query and proceeding"
			<< "...";
		m_proof_state = ProofProcessState::RUNNING_PROOF;
		m_temp_results = std::stringstream();  // reset this
		m_db_op.reset();  // and this
		break;
	}
}


void ProofProcess::save_results()
{
	ATP_CORE_ASSERT(m_db_op != nullptr);

	switch (m_db_op->state())
	{
	case atp::db::TransactionState::RUNNING:
		m_db_op->step();
		break;

	case atp::db::TransactionState::COMPLETED:
		m_done = true;
		m_db_op.reset();
		ATP_CORE_LOG(info) << "Proof Process update --- "
			<< "Finished saving the true theorems to the database"
			<< ", so they can be used in future proofs!";
		break;

	default:
		ATP_CORE_LOG(error) << "Failed to save proof results. Unexpected "
			<< "error while executing query. Cancelling...";
		m_done = true;
		m_db_op.reset();
		break;
	}
}


void ProofProcess::setup_init_kernel_operation()
{
	ATP_CORE_ASSERT(m_db_op == nullptr);

	ATP_CORE_LOG(trace) << "Setting up kernel initialisation query...";

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::RANDOM_PROVEN_THM_SELECTION);

	auto p_bder = dynamic_cast<
		atp::db::IRndProvenThmSelectQryBder*>(_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_limit(25  /* todo: don't hardcode */)
		->set_context(m_ctx_id, m_ctx);

	const auto query = p_bder->build();

	m_db_op = m_db->begin_transaction(query);

	ATP_CORE_ASSERT(m_db_op != nullptr);
}


void ProofProcess::setup_save_results_operation()
{
	ATP_CORE_ASSERT(m_db_op == nullptr);

	ATP_CORE_LOG(trace) << "Setting up result-saving query...";

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

	auto p_bder = dynamic_cast<
		atp::db::ISaveProofResultsQryBder*>(_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

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

	ATP_CORE_ASSERT(m_db_op != nullptr);
}


}  // namespace core
}  // namespace atp


