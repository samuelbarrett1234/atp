#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of a process which runs an
	automated conjecturing procedure.
*/


#include <sstream>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "../Models/HMMConjectureModel.h"
#include "../Models/HMMConjectureModelBuilder.h"


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
class ATP_CORE_API HMMConjectureProcess :
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

	/**
	\brief We generate conjectures in a recursive manner, hence
		bundle up all of the data we need to put in the stack into
		one struct.
	*/
	struct ATP_CORE_API CurrentStmtStackFrame
	{
		// observation parameters given by HMM model generation
		const size_t cur_obs, cur_obs_arity;
		const bool cur_obs_is_free;

		// a string representing the current expression so far (as
		// its children are completed, they will be appended here).
		std::stringstream done_so_far;

		// the next child that needs to be converted (which will be
		// appearing immediately above this entry on the stack).
		// as soon as cur_idx == cur_obs_arity, this will be ready to
		// pop from the stack
		size_t cur_idx;
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
	inline bool waiting() const override
	{
		return m_db_op != nullptr && m_db_op->waiting();
	}
	void run_step() override;

private:
	// in order of execution:

	void setup_find_model_transaction();
	void load_model_results();
	bool check_model_loaded();  // returns true iff success

	void setup_get_state_trans_params();
	void load_state_trans_results();

	void setup_get_obs_params();
	void load_obs_params_results();

	bool construct_model();  // returns true iff success

	void setup_conjecture_generation();
	void generate_a_conjecture();

	void setup_save_results();

	// helper function for `generate_a_conjecture`
	// this can fail, but on rare occasions
	boost::optional<std::string> generate_expression();

private:
	const db::DatabasePtr m_db;
	const logic::LanguagePtr m_lang;
	const logic::ModelContextPtr m_ctx;
	const size_t m_ctx_id;
	const size_t m_num_to_gen;
	HMMConjProcState m_state;

	// this might be None from the constructor, but we will fill it
	// in later if that's the case.
	boost::optional<size_t> m_model_id;

	// we will gradually put model params into here, when in the
	// right state
	boost::optional<HMMConjectureModelBuilder> m_model_builder;

	// once model is built, it will be put here:
	std::unique_ptr<HMMConjectureModel> m_model;

	// the current database operation (may be null)
	db::TransactionPtr m_db_op;

	// committed results (completed statements)
	std::vector<std::string> m_completed;
};


}  // namespace core
}  // namespace atp


