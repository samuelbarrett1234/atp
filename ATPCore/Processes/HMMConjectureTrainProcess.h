#pragma once


/**
\file

\author Samuel Barrett

*/


#include <vector>
#include <sstream>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "../Models/HMMConjectureModelBuilder.h"


namespace atp
{
namespace core
{


class ATP_CORE_API HMMConjectureTrainProcess :
	public IProcess
{
private:
	enum class HMMConjTrainState
	{
		FINDING_MODEL,
		GETTING_STATE_TRANSITION_PARAMS,
		GETTING_OBSERVATION_PARAMS,
		GETTING_STATEMENTS_TO_TRAIN,
		TRAINING,
		SAVING_RESULTS,
		DONE, FAILED
	};
public:
	/**
	\param num_epochs The number of training iterations to perform
		over the dataset.

	\param max_dataset_size The limit on how many statements can be
		loaded from the database to train on.

	\param model_id The model ID to look for, to train (optional).

	\pre num_epochs > 0 && max_dataset_size > 0

	*/
	HMMConjectureTrainProcess(db::DatabasePtr p_db,
		logic::LanguagePtr p_lang, size_t ctx_id,
		logic::ModelContextPtr p_ctx, size_t num_epochs,
		size_t max_dataset_size,
		boost::optional<size_t> model_id = boost::none);

	inline bool done() const override
	{
		return m_state == HMMConjTrainState::DONE
			|| m_state == HMMConjTrainState::FAILED;
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

	bool construct_model_trainer();  // returns true iff success

	void setup_get_statements_transaction();
	void load_statements();
	bool finish_loading_statements();

	void train_one();

	void setup_save_results();

private:
	const db::DatabasePtr m_db;
	const logic::LanguagePtr m_lang;
	const logic::ModelContextPtr m_ctx;
	const size_t m_ctx_id;
	const size_t m_num_epochs, m_max_dataset_size;
	size_t m_epochs_so_far;
	HMMConjTrainState m_state;
	const std::vector<size_t> m_symbols;

	std::stringstream m_stmt_strs;
	logic::StatementArrayPtr m_stmts;
	boost::optional<HMMConjectureModelBuilder> m_model_builder;
	std::unique_ptr<HMMConjectureModel> m_model;

	boost::optional<size_t> m_model_id;

	db::TransactionPtr m_db_op;
};


}  // namespace core
}  // namespace atp


