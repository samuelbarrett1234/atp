#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of a process which runs an
	automated conjecturing procedure.
*/


#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


/**
\brief This is a process which will run an automated conjecturing
	procedure continuously.

\warning This HMMConjectureProcess will try to load a specific model
	from the database. If there are no models found, the process
	will log an error and exit. If there are multiple models that
	could be selected for a given model context, and one was not
	provided in the constructor, we will issue a warning and select
	one arbitrarily.
*/
class HMMConjectureProcess :
	public IProcess
{
private:
	/**
	\brief the different states this process can be in (listed in
		order).
	*/
	enum class HMMConjProcState
	{
		FINDING_MODEL,
		GETTING_STATE_TRANSITION_PARAMS,
		GETTING_OBSERVATION_PARAMS,
		GENERATING_CONJECTURES,
		SAVING_RESULTS,
		DONE, FAILED
	};

public:
	/**
	\brief Create a new process to generate conjectures

	\param num_to_generate The number of conjectures to generate

	\param model_id Optionally, you can reference a specific model
		ID from the database, otherwise one is selected arbitrarily.
	*/
	HMMConjectureProcess(atp::db::DatabasePtr p_db,
		logic::LanguagePtr p_lang, size_t ctx_id,
		logic::ModelContextPtr p_ctx, size_t num_to_generate,
		boost::optional<size_t> model_id = boost::none);

	inline bool done() const override
	{
		return m_state == HMMConjProcState::DONE
			|| m_state == HMMConjProcState::FAILED;
	}
	bool waiting() const override;
	void run_step() override;

private:
	void setup_find_model_transaction();
	void load_model_results();

	void setup_get_state_trans_params();
	void load_state_trans_results();

	void setup_get_obs_params();
	void load_obs_params_results();

private:
	db::DatabasePtr m_db;
	logic::LanguagePtr m_lang;
	const logic::ModelContextPtr m_ctx;
	const size_t m_ctx_id;
	HMMConjProcState m_state;

	// this might be None from the constructor, but we will fill it
	// in later if that's the case.
	boost::optional<size_t> m_model_id;

	// we will gradually put model params into here
	HMMConjectureModelBuilder m_model_builder;

	// the current database operation (may be null)
	db::TransactionPtr m_db_op;

	// committed results (completed statements)
	std::vector<std::string> m_completed;
};


}  // namespace core
}  // namespace atp


