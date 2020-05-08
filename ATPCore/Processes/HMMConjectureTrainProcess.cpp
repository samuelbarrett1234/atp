/**
\file

\author Samuel Barrett

*/


#include <chrono>
#include <boost/optional/optional_io.hpp>
#include "HMMConjectureTrainProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


// get all symbol IDs from model context
static std::vector<size_t> get_all_symbols(
	const logic::ModelContextPtr& p_ctx);


HMMConjectureTrainProcess::HMMConjectureTrainProcess(
	db::DatabasePtr p_db, logic::LanguagePtr p_lang,
	size_t ctx_id, logic::ModelContextPtr p_ctx,
	size_t num_epochs, size_t max_dataset_size,
	boost::optional<size_t> model_id) :
	m_db(std::move(p_db)), m_lang(std::move(p_lang)),
	m_ctx(std::move(p_ctx)), m_ctx_id(ctx_id),
	m_num_epochs(num_epochs), m_max_dataset_size(max_dataset_size),
	m_model_id(model_id), m_model_builder(m_ctx), m_state(
		HMMConjTrainState::FINDING_MODEL),
	m_epochs_so_far(0), m_symbols(get_all_symbols(m_ctx))
{
	ATP_CORE_PRECOND(m_num_epochs > 0);
	ATP_CORE_PRECOND(m_max_dataset_size > 0);
	ATP_CORE_PRECOND(m_db != nullptr);
	ATP_CORE_PRECOND(m_lang != nullptr);
	ATP_CORE_PRECOND(m_ctx != nullptr);

	// initial state requires this object
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// seed the conjecture generator with the current time
	const size_t seed = (size_t)
		std::chrono::high_resolution_clock
		::now().time_since_epoch().count();
	ATP_CORE_LOG(debug) << "Seeded the HMM model with " << seed;
	m_model_builder->set_random_seed(seed);

	setup_find_model_transaction();
}


void HMMConjectureTrainProcess::run_step()
{
	if (m_db_op != nullptr)
	{
		m_db_op->step();

		switch (m_db_op->state())
		{
		case db::TransactionState::RUNNING:
			// dispatch to load_..._results()
			switch (m_state)
			{
			case HMMConjTrainState::FINDING_MODEL:
				ATP_CORE_LOG(trace) << "Searching for HMM model...";
				load_model_results();
				break;

			case HMMConjTrainState::GETTING_STATE_TRANSITION_PARAMS:
				ATP_CORE_LOG(trace) << "Loading HMM model state "
					"transition parameters...";
				load_state_trans_results();
				break;

			case HMMConjTrainState::GETTING_OBSERVATION_PARAMS:
				ATP_CORE_LOG(trace) << "Loading HMM model "
					"observation parameters...";
				load_obs_params_results();
				break;

			case HMMConjTrainState::GETTING_STATEMENTS_TO_TRAIN:
				ATP_CORE_LOG(trace) << "Loading statements to train "
					"HMM on...";
				load_statements();
				break;

				// default: do nothing
			}
			break;
		case db::TransactionState::FAILED:
			// bail out
			ATP_CORE_LOG(error) << "HMMConjectureTrainProcess DB "
				"operation failed, bailing...";
			m_state = HMMConjTrainState::FAILED;
			m_db_op.reset();
			break;
		case db::TransactionState::COMPLETED:
			// finish operation and transition state:
			m_db_op.reset();

			switch (m_state)
			{
			case HMMConjTrainState::FINDING_MODEL:
				ATP_CORE_LOG(trace) << "Finished finding HMM model.";
				m_state = HMMConjTrainState::
					GETTING_STATE_TRANSITION_PARAMS;
				if (check_model_loaded())
					setup_get_state_trans_params();
				break;

			case HMMConjTrainState::GETTING_STATE_TRANSITION_PARAMS:
				ATP_CORE_LOG(trace) << "Finished getting HMM model "
					"state transition parameters.";
				m_state = HMMConjTrainState::
					GETTING_OBSERVATION_PARAMS;
				setup_get_obs_params();
				break;

			case HMMConjTrainState::GETTING_OBSERVATION_PARAMS:
				ATP_CORE_LOG(trace) << "Finished getting HMM model "
					"observation parameters.";
				m_state = HMMConjTrainState::
					GETTING_STATEMENTS_TO_TRAIN;
				if (construct_model_trainer())
					setup_get_statements_transaction();
				break;

			case HMMConjTrainState::GETTING_STATEMENTS_TO_TRAIN:
				ATP_CORE_LOG(trace) << "Finished getting statements "
					"to train HMM on.";
				if (finish_loading_statements())
					m_state = HMMConjTrainState::TRAINING;
				break;

			case HMMConjTrainState::SAVING_RESULTS:
				ATP_CORE_LOG(trace) << "Finished saving conjectures "
					"produced by HMM model!";
				m_state = HMMConjTrainState::
					DONE;
				break;

			default:
				ATP_CORE_ASSERT(false && "bad state.");
			}
			break;
		}
	}
	// this state is special because it isn't a DB transaction
	else if (m_state == HMMConjTrainState::TRAINING)
	{
		train_one();

		if (m_epochs_so_far == m_num_epochs)
		{
			ATP_CORE_LOG(info) << "Finished training HMM model for "
				<< m_num_epochs << " epochs. Saving results to "
				"database...";

			// we are done, transition to next state:

			m_state = HMMConjTrainState::SAVING_RESULTS;

			// get rid of stuff we don't need anymore:
			m_model.reset();

			// initialise next section of the operation
			setup_save_results();
		}
	}
}


void HMMConjectureTrainProcess::setup_find_model_transaction()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::FINDING_MODEL);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	// this is set in the constructor:
	ATP_CORE_ASSERT(m_model_builder.has_value());

	auto _p_bder = m_db->create_query_builder(
		db::QueryBuilderType::FIND_HMM_CONJ_MODEL);

	auto p_bder = dynamic_cast<db::IFindHmmConjModel*>(
		_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_ctx(m_ctx_id, m_ctx);

	if (m_model_id.has_value())
		p_bder->set_model_id(*m_model_id);

	const auto query = p_bder->build();
	m_db_op = m_db->begin_transaction(query);

	if (m_db_op == nullptr)
	{
		ATP_CORE_LOG(error) << "Failed to run query: \""
			<< query << "\", perhaps the tables aren't initialised"
			" for the HMM conjecturer? Model context ID was " <<
			m_ctx_id << "(" << m_ctx->context_name() << ") and "
			"ID searched for was " << m_model_id;

		m_state = HMMConjTrainState::FAILED;
	}
}


void HMMConjectureTrainProcess::load_model_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::FINDING_MODEL);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	// this is set in the constructor:
	ATP_CORE_ASSERT(m_model_builder.has_value());

	auto p_query = dynamic_cast<db::IQueryTransaction*>(m_db_op.get());

	ATP_CORE_ASSERT(p_query != nullptr);

	// the query is not obliged to have values
	if (p_query->has_values())
	{
		// this should be an assert, because if the database did not
		// contain the three columns we are looking for, the query
		// would not have been built in the first place and we would
		// have caught the error.
		ATP_CORE_ASSERT(p_query->arity() == 3);

		db::DValue id,
			num_states,
			free_q;

		if (!p_query->try_get(0, db::DType::INT, &id))
		{
			ATP_CORE_LOG(error) << "Query to obtain HMM conjecturer "
				"model data failed - query did not return an ID.";
			m_state = HMMConjTrainState::FAILED;
			m_db_op.reset();
		}
		m_model_id = (size_t)db::get_int(id);

		if (p_query->try_get(1, db::DType::INT, &num_states))
		{
			m_model_builder->set_num_hidden_states(
				(size_t)db::get_int(num_states));
		}

		if (p_query->try_get(2, db::DType::FLOAT, &free_q))
		{
			m_model_builder->set_free_geometric_q(
				db::get_float(free_q));
		}
	}
}


bool HMMConjectureTrainProcess::check_model_loaded()
{
	// the existence of this is sufficient to proceed loading the
	// rest of the model
	// if the other parameters (free_q and num_states) were invalid
	// they will be caught later.
	if (!m_model_id.has_value())
	{
		m_state = HMMConjTrainState::FAILED;
	}
	return m_model_id.has_value();
}


void HMMConjectureTrainProcess::setup_get_state_trans_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::
		GETTING_STATE_TRANSITION_PARAMS);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());
	ATP_CORE_ASSERT(m_model_id.has_value());  // should've been set

	auto _p_bder = m_db->create_query_builder(
		db::QueryBuilderType::GET_HMM_CONJ_ST_TRANS_PARAMS);

	auto p_bder = dynamic_cast<db::IGetHmmConjectureModelParams*>(
		_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_ctx(m_ctx_id, m_ctx)
		->set_model_id(*m_model_id);

	const auto query = p_bder->build();
	m_db_op = m_db->begin_transaction(query);

	if (m_db_op == nullptr)
	{
		ATP_CORE_LOG(error) << "Failed to run query: \""
			<< query << "\", perhaps the tables aren't initialised"
			" for the HMM conjecturer? Model context ID was " <<
			m_ctx_id << " (" << m_ctx->context_name() << ") and "
			"the model ID searched for was " << *m_model_id;

		m_state = HMMConjTrainState::FAILED;
	}
}


void HMMConjectureTrainProcess::load_state_trans_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::
		GETTING_STATE_TRANSITION_PARAMS);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	auto p_query = dynamic_cast<db::IQueryTransaction*>(m_db_op.get());

	ATP_CORE_ASSERT(p_query != nullptr);

	// query not obliged to have values
	if (p_query->has_values())
	{
		db::DValue pre_state, post_state, prob;

		if (p_query->try_get(0, db::DType::INT, &pre_state)
			&& p_query->try_get(1, db::DType::INT, &post_state)
			&& p_query->try_get(2, db::DType::FLOAT, &prob))
		{
			m_model_builder->add_state_transition(
				(size_t)db::get_int(pre_state),
				(size_t)db::get_int(post_state),
				db::get_float(prob));
		}
	}
}


void HMMConjectureTrainProcess::setup_get_obs_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::
		GETTING_OBSERVATION_PARAMS);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());
	ATP_CORE_ASSERT(m_model_id.has_value());  // should've been set

	auto _p_bder = m_db->create_query_builder(
		db::QueryBuilderType::GET_HMM_CONJ_OBS_PARAMS);

	auto p_bder = dynamic_cast<db::IGetHmmConjectureModelParams*>(
		_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_ctx(m_ctx_id, m_ctx)
		->set_model_id(*m_model_id);

	const auto query = p_bder->build();
	m_db_op = m_db->begin_transaction(query);

	if (m_db_op == nullptr)
	{
		ATP_CORE_LOG(error) << "Failed to run query: \""
			<< query << "\", perhaps the tables aren't initialised"
			" for the HMM conjecturer? Model context ID was " <<
			m_ctx_id << " (" << m_ctx->context_name() << ") and "
			"the model ID searched for was " << *m_model_id;

		m_state = HMMConjTrainState::FAILED;
	}
}


void HMMConjectureTrainProcess::load_obs_params_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::
		GETTING_OBSERVATION_PARAMS);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	auto p_query = dynamic_cast<db::IQueryTransaction*>(m_db_op.get());

	ATP_CORE_ASSERT(p_query != nullptr);

	// query not obliged to have values
	if (p_query->has_values())
	{
		db::DValue state, obs, prob;

		if (p_query->try_get(0, db::DType::INT, &state)
			&& p_query->try_get(1, db::DType::STR, &obs)
			&& p_query->try_get(2, db::DType::FLOAT, &prob))
		{
			m_model_builder->add_symbol_observation(
				(size_t)db::get_int(state),
				db::get_str(obs),
				db::get_float(prob));
		}
	}
}


bool HMMConjectureTrainProcess::construct_model_trainer()
{
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());
	ATP_CORE_ASSERT(m_epochs_so_far == 0);
	ATP_CORE_ASSERT(m_stmts == nullptr);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::TRAINING);
	ATP_CORE_ASSERT(m_model_id.has_value());

	if (m_model_builder->can_build())
	{
		m_model = m_model_builder->build();

		ATP_CORE_LOG(trace) << "Built HMM from loaded params!";

		return true;
	}
	else
	{
		ATP_CORE_LOG(error) << "Failed to construct model! Some or "
			"all parameters were incorrect. Please check HMM "
			"conjecturer model with ID " << *m_model_id;

		m_state = HMMConjTrainState::FAILED;
		m_model_builder.reset();

		return false;
	}
}


void HMMConjectureTrainProcess::setup_get_statements_transaction()
{
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_state ==
		HMMConjTrainState::GETTING_STATEMENTS_TO_TRAIN);

	ATP_CORE_LOG(trace) << "Setting up query to get proven theorems"
		" from the database to train on.";

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

	auto p_bder = dynamic_cast<
		atp::db::IRndThmSelectQryBder*>(_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_limit(m_max_dataset_size)
		->set_context(m_ctx_id, m_ctx)
		->set_proven(true);  // DEFINITELY load proven statements!
	
	const auto query = p_bder->build();

	m_db_op = m_db->begin_transaction(query);

	ATP_CORE_ASSERT(m_db_op != nullptr);
}


void HMMConjectureTrainProcess::load_statements()
{
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_state ==
		HMMConjTrainState::GETTING_STATEMENTS_TO_TRAIN);

	auto p_readable_op = dynamic_cast<atp::db::IQueryTransaction*>
		(m_db_op.get());

	ATP_CORE_ASSERT(p_readable_op != nullptr);

	if (p_readable_op->has_values())
	{
		if (p_readable_op->arity() != 1)
		{
			ATP_CORE_LOG(error) << "Failed to initialise kernel."
				" Kernel initialisation query returned an arity "
				"of " << p_readable_op->arity() << ", which "
				"differed from the expected result of 1. "
				"Ignoring query...";
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
				ATP_CORE_LOG(debug) << "Adding '"
					<< atp::db::get_str(stmt_str)
					<< "' to helper theorems.";

				m_stmt_strs << atp::db::get_str(stmt_str)
					<< std::endl;
			}
		}
	}

}


bool HMMConjectureTrainProcess::finish_loading_statements()
{
	ATP_CORE_ASSERT(m_state ==
		HMMConjTrainState::GETTING_STATEMENTS_TO_TRAIN);
	ATP_CORE_ASSERT(m_stmts == nullptr);

	m_stmts = m_lang->deserialise_stmts(m_stmt_strs,
		atp::logic::StmtFormat::TEXT, *m_ctx);

	if (m_stmts == nullptr)
	{
		ATP_CORE_LOG(error) <<
			"Failed to get dataset for training. There was"
			" a problem with the following batch of "
			"statements retrieved from the database: \""
			<< m_stmt_strs.str() << '"';
	}
	else if (m_stmts->size() > 0)
	{
		ATP_CORE_LOG(info) << "HMM Train Process: "
			"Loaded dataset of size" << m_stmts->size()
			<< " from the theorem database!";
	}
	else
	{
		ATP_CORE_LOG(warning) <<
			"HMM train process could not find any "
			"theorems to load from the database.";

		// finish immediately
		m_state = HMMConjTrainState::DONE;
		return true;
	}

	m_stmt_strs = std::stringstream();  // reset this
	m_db_op.reset();  // and this
}


void HMMConjectureTrainProcess::train_one()
{
	ATP_CORE_ASSERT(m_model != nullptr);
	ATP_CORE_ASSERT(m_epochs_so_far < m_num_epochs);
	ATP_CORE_ASSERT(m_stmts != nullptr);
	ATP_CORE_ASSERT(!m_model_builder.has_value());
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_state == HMMConjTrainState::TRAINING);

	// limit the number of epochs to perform in a single step
	static const size_t num_epochs_per_step = 10;

	const size_t epochs_todo_now = std::min(
		num_epochs_per_step, m_num_epochs - m_epochs_so_far);

	m_model->train(m_stmts, epochs_todo_now);

	m_epochs_so_far += m_num_epochs;
}


std::vector<size_t> get_all_symbols(
	const logic::ModelContextPtr& p_ctx)
{
	auto result = p_ctx->all_constant_symbol_ids();

	auto temp = p_ctx->all_function_symbol_ids();

	result.insert(result.end(), temp.begin(), temp.end());

	return result;
}


}  // namespace core
}  // namespace atp


