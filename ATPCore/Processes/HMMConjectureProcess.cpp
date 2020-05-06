/**
\file

\author Samuel Barrett

*/


#include <boost/algorithm/string/join.hpp>
#include "HMMConjectureProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


HMMConjectureProcess::HMMConjectureProcess(
	atp::db::DatabasePtr p_db, logic::LanguagePtr p_lang,
	size_t ctx_id, logic::ModelContextPtr p_ctx,
	size_t num_to_generate, boost::optional<size_t> model_id) :
	m_db(std::move(p_db)), m_lang(std::move(p_lang)),
	m_ctx(std::move(p_ctx)), m_ctx_id(ctx_id),
	m_num_to_gen(num_to_generate), m_model_id(model_id),
	m_model_builder(m_ctx), m_state(HMMConjProcState::FINDING_MODEL)
{
	ATP_CORE_PRECOND(num_to_generate > 0);
	ATP_CORE_PRECOND(m_db != nullptr);
	ATP_CORE_PRECOND(m_lang != nullptr);
	ATP_CORE_PRECOND(m_ctx != nullptr);

	// initial state requires this object
	ATP_CORE_ASSERT(m_model_builder.has_value());

	setup_find_model_transaction();
}


void HMMConjectureProcess::run_step()
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
			case HMMConjProcState::FINDING_MODEL:
				load_model_results();
				break;
				
			case HMMConjProcState::GETTING_STATE_TRANSITION_PARAMS:
				load_state_trans_results();
				break;

			case HMMConjProcState::GETTING_OBSERVATION_PARAMS:
				load_obs_params_results();
				break;

				// default: do nothing
			}
			break;
		case db::TransactionState::FAILED:
			// bail out
			ATP_CORE_LOG(error) << "HMMConjectureProcess DB "
				"operation failed, bailing...";
			m_state = HMMConjProcState::FAILED;
			m_db_op.reset();
			break;
		case db::TransactionState::COMPLETED:
			// finish operation and transition state:
			m_db_op.reset();

			switch (m_state)
			{
			case HMMConjProcState::FINDING_MODEL:
				m_state = HMMConjProcState::
					GETTING_STATE_TRANSITION_PARAMS;
				if (check_model_loaded())
					setup_get_state_trans_params();
				break;

			case HMMConjProcState::GETTING_STATE_TRANSITION_PARAMS:
				m_state = HMMConjProcState::
					GETTING_OBSERVATION_PARAMS;
				setup_get_obs_params();
				break;

			case HMMConjProcState::GETTING_OBSERVATION_PARAMS:
				m_state = HMMConjProcState::
					GENERATING_CONJECTURES;
				if (construct_model())
					setup_conjecture_generation();
				break;

			case HMMConjProcState::SAVING_RESULTS:
				m_state = HMMConjProcState::
					DONE;
				break;

			default:
				ATP_CORE_ASSERT(false && "bad state.");
			}
			break;
		}
	}
	// this state is special because it isn't a DB transaction
	else if (m_state == HMMConjProcState::GENERATING_CONJECTURES)
	{
		generate_a_conjecture();

		if (m_completed.size() == m_num_to_gen)
		{
			// we are done, transition to next state:

			m_state = HMMConjProcState::SAVING_RESULTS;

			// get rid of stuff we don't need anymore:
			m_model.reset();

			// initialise next section of the operation
			setup_save_results();
		}
	}
}


void HMMConjectureProcess::setup_find_model_transaction()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::FINDING_MODEL);
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

		m_state = HMMConjProcState::FAILED;
	}
}


void HMMConjectureProcess::load_model_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::FINDING_MODEL);
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
			m_state = HMMConjProcState::FAILED;
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


bool HMMConjectureProcess::check_model_loaded()
{
	// the existence of this is sufficient to proceed loading the
	// rest of the model
	// if the other parameters (free_q and num_states) were invalid
	// they will be caught later.
	if (!m_model_id.has_value())
	{
		m_state = HMMConjProcState::FAILED;
	}
	return m_model_id.has_value();
}


void HMMConjectureProcess::setup_get_state_trans_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
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
			m_ctx_id << "(" << m_ctx->context_name() << ") and "
			"ID searched for was " << *m_model_id;

		m_state = HMMConjProcState::FAILED;
	}
}


void HMMConjectureProcess::load_state_trans_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
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


void HMMConjectureProcess::setup_get_obs_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
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
			m_ctx_id << "(" << m_ctx->context_name() << ") and "
			"ID searched for was " << *m_model_id;

		m_state = HMMConjProcState::FAILED;
	}
}


void HMMConjectureProcess::load_obs_params_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
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
			&& p_query->try_get(1, db::DType::INT, &obs)
			&& p_query->try_get(2, db::DType::FLOAT, &prob))
		{
			m_model_builder->add_symbol_observation(
				(size_t)db::get_int(state),
				(size_t)db::get_int(obs),
				db::get_float(prob));
		}
	}
}


bool HMMConjectureProcess::construct_model()
{
	ATP_CORE_ASSERT(m_model_id.has_value());

	if (m_model_builder->can_build())
	{
		m_model = m_model_builder->build();

		ATP_CORE_LOG(trace) << "Built HMM conjecturer model!";
	}
	else
	{
		ATP_CORE_LOG(error) << "Failed to construct model! Some or "
			"all parameters were incorrect. Please check HMM "
			"conjecturer model with ID " << *m_model_id;
		m_state = HMMConjProcState::FAILED;
	}
	m_model_builder.reset();
}


void HMMConjectureProcess::setup_conjecture_generation()
{
	ATP_CORE_ASSERT(m_model_builder.has_value());
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GENERATING_CONJECTURES);
	ATP_CORE_ASSERT(m_model_id.has_value());

	// check parameters are valid first (they may not be, if data
	// loaded from DB was rubbish)
	if (m_model_builder->can_build())
	{
		// create model
		m_model = m_model_builder->build();
	}
	else
	{
		ATP_CORE_LOG(error) << "Failed to create HMM conjecture-"
			"generation model, because the parameters loaded from "
			"the database were bad. Model ID was " << *m_model_id
			<< ". Stopping conjecture process...";

		m_state = HMMConjProcState::FAILED;
	}

	m_model_builder.reset();  // don't need anymore
}


void HMMConjectureProcess::generate_a_conjecture()
{
	ATP_CORE_ASSERT(m_model != nullptr);
	ATP_CORE_ASSERT(!m_model_builder.has_value());
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_completed.size() < m_num_to_gen);
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GENERATING_CONJECTURES);

	// reset state to default, before generating a new statement
	m_model->reset_state();

	// it is important that they are evaluated in this order, as the
	// HMM state is passed between them.
	auto expr1 = generate_expression(),
		expr2 = generate_expression();

	if (expr1.has_value() && expr2.has_value())
	{
		m_completed.push_back(*expr1 + " = " + *expr2);

		ATP_CORE_LOG(trace) << "HMMConjectureProcess just generated the "
			"statement \"" << m_completed.back() << "\".";
	}
	else
	{
		ATP_CORE_LOG(warning) << "Failed to generate conjecture.";
	}

	// TODO: there are a few ways we could use a HMM to generate
	// these statements; in particular, in the way we let the
	// state transition. Is there a nice way of selecting between
	// them? For now we will only implement the one below.
}


void HMMConjectureProcess::setup_save_results()
{
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_completed.size() == m_num_to_gen);
	ATP_CORE_ASSERT(m_state == HMMConjProcState::SAVING_RESULTS);

	// load conjectures as statement objects

	const auto conjs = boost::algorithm::join(m_completed,
		"\n");
	logic::StatementArrayPtr p_targets;
	{
		std::stringstream s(conjs);
		p_targets = m_lang->deserialise_stmts(s,
			logic::StmtFormat::TEXT, *m_ctx);
	}

	if (p_targets == nullptr)
	{
		ATP_CORE_LOG(error) << "Failed to parse generated "
			"conjectures! Conjectures were: \"" << conjs << "\".";

		m_state = HMMConjProcState::FAILED;
		return;
	}

	// now create a query to save them

	auto _p_bder = m_db->create_query_builder(
		db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

	auto p_bder = dynamic_cast<db::ISaveProofResultsQryBder*>(
		_p_bder.get());

	ATP_CORE_ASSERT(p_bder != nullptr);

	p_bder->set_context(m_ctx_id, m_ctx)
		->add_target_thms(p_targets);

	const auto query = p_bder->build();
	m_db_op = m_db->begin_transaction(query);

	if (m_db_op == nullptr)
	{
		ATP_CORE_LOG(error) << "Failed to create query to save "
			"conjectures to the database! Conjectures were: \""
			<< conjs << "\".";

		m_state = HMMConjProcState::FAILED;
	}
}


boost::optional<std::string> HMMConjectureProcess::generate_expression()
{
	// to ensure termination, create an iteration limit:
	static const size_t ITERATION_LIMIT = 10000;

	ATP_CORE_ASSERT(m_model != nullptr);

	// do not reset model state here!

	std::vector<CurrentStmtStackFrame> stack;

	auto add_cur_model_obs = [&stack, this]()
	{
		stack.emplace_back(CurrentStmtStackFrame{
			m_model->current_observation(),
			m_model->current_observation_arity(),
			m_model->current_observation_is_free_var(),
			std::stringstream(), 0
			});

		if (m_model->current_observation_is_free_var())
		{
			stack.back().done_so_far << 'x' <<
				m_model->current_observation();
		}
		else
		{
			stack.back().done_so_far << m_ctx->symbol_name(
				m_model->current_observation());

			if (m_model->current_observation_arity() > 0)
			{
				stack.back().done_so_far << "(";
			}
		}
	};

	// push first element
	add_cur_model_obs();

	size_t iters = 0;
	while (stack.front().cur_idx < stack.front().cur_obs_arity
		&& iters < ITERATION_LIMIT)
	{
		++iters;

		// handle back element (top of stack)
		if (stack.back().cur_idx < stack.back().cur_obs_arity)
		{
			// advance the model (to update the state):
			m_model->advance();

			add_cur_model_obs();
		}
		else if (stack.size() > 1)
		{
			auto elem = std::move(stack.back());
			stack.pop_back();

			// advance next child and add our string to it:
			++stack.back().cur_idx;
			stack.back().done_so_far << elem.done_so_far.str();

			// add comma or closing bracket
			if (stack.back().cur_idx < stack.back().cur_obs_arity)
			{
				stack.back().done_so_far << ", ";
			}
			else
			{
				stack.back().done_so_far << ")";
			}
		}
	}

	if (iters == ITERATION_LIMIT)
	{
		ATP_CORE_LOG(error) << "Iteration limit reached in generating "
			"conjecture from HMMConjectureProcess, indicating bad "
			"model parameters. Stack size was " << stack.size() <<
			".";
		return boost::none;
	}
	return stack.front().done_so_far.str();
}


}  // namespace core
}  // namespace atp


