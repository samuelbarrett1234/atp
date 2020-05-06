/**
\file

\author Samuel Barrett

*/


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

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::load_model_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::FINDING_MODEL);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	// this is set in the constructor:
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::setup_get_state_trans_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GETTING_STATE_TRANSITION_PARAMS);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::load_state_trans_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GETTING_STATE_TRANSITION_PARAMS);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::setup_get_obs_params()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GETTING_OBSERVATION_PARAMS);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::load_obs_params_results()
{
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GETTING_OBSERVATION_PARAMS);
	ATP_CORE_ASSERT(m_db_op != nullptr);
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_model_builder.has_value());

	// TODO
	ATP_CORE_ASSERT(false);
}


void HMMConjectureProcess::setup_conjecture_generation()
{
	ATP_CORE_ASSERT(m_model_builder.has_value());
	ATP_CORE_ASSERT(m_model == nullptr);
	ATP_CORE_ASSERT(m_db_op == nullptr);
	ATP_CORE_ASSERT(m_state == HMMConjProcState::
		GENERATING_CONJECTURES);

	// check parameters are valid first (they may not be, if data
	// loaded from DB was rubbish)
	if (m_model_builder->can_build())
	{
		// create model
		m_model = m_model_builder->build();

		m_model_builder.reset();  // don't need anymore
	}
	else
	{
		ATP_CORE_LOG(error) << "Failed to create HMM conjecture-"
			"generation model, because the parameters loaded from "
			"the database were bad. Stopping conjecture process...";

		m_model_builder = boost::none;
		m_state = HMMConjProcState::FAILED;
	}
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

	m_completed.push_back(expr1 + " = " + expr2);

	ATP_CORE_LOG(trace) << "HMMConjectureProcess just generated the "
		"statement \"" << m_completed.back() << "\".";

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

	// TODO
	ATP_CORE_ASSERT(false);
}


std::string HMMConjectureProcess::generate_expression()
{
	ATP_CORE_ASSERT(m_model != nullptr);

	// do not reset model state here!

	std::vector<CurrentStmtStackFrame> stack;

	// push first element
	stack.push_back(CurrentStmtStackFrame{
		m_model->current_observation(),
		m_model->current_observation_arity(),
		m_model->current_observation_is_free_var(),
		std::stringstream(), 0
		});

	while (stack.front().cur_idx < stack.front().cur_obs_arity)
	{
		// firstly, advance the model (to update the state):
		m_model->advance();

		// now we need to handle the back element.

		// check invariant:
		ATP_CORE_ASSERT(stack.back().cur_idx <
			stack.back().cur_obs_arity);

		// TODO: implement the rest of this!!!
		ATP_CORE_ASSERT(false);
	}

	return stack.front().done_so_far.str();
}


}  // namespace core
}  // namespace atp


