#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the simple command line application for doing a proof

*/


#include "ATP.h"
#include <string>
#include <vector>
#include <ATPLogic.h>
#include <ATPSearch.h>
#include <ATPDatabase.h>
#include <ATPCore.h>


/**

\brief The simple command-line application for proving a small set of
    statements.

\details This class encapsulates the following use case: "given a
    logical language, and a context, try to prove a given set of
	statements all in one go." It does not store the results in the
	database; this application is intended to be quick and simple.
*/
class Application
{
public:
	/**
	\param num_threads The number of threads to devote to execution.

	\pre num_threads > 0
	*/
	Application(size_t num_threads);

	/**
	\brief Loads the database file (this has to be done first)

	\returns True iff success.
	*/
	bool set_db(const std::string& db_config_file);

	/**
	\brief Loads the language and knowledge kernel from a given
	    context (filename obtained from database).
		
	\details A context file specifies the language, and the
		definitions and axioms.

	\returns True iff success.
	*/
	bool set_context_name(const std::string& name);

	/**
	\brief Loads the solver

	\pre The context file was successfully loaded.

	\returns True iff success.
	*/
	bool set_search_name(const std::string& path);

	/**
	\brief Add a process which will try to prove the given statements
		when `run` is called.

	\param path_or_stmt Can either be a path to a file of statements
	    or can just be a list of statements represented as a string
		(in both cases, multiple statements should be line-separated)

	\returns True iff success.

	\pre Language, context file, search settings and database must be
		initialised.

	*/
	bool add_proof_task(std::string path_or_stmt);

	/**
	\brief Add a process which will select some arbitrary unproven
		statements and try to prove them when `run` is called.

	\param num_targets The number of unproven theorems to try proving

	\returns True iff success.

	\pre Language, context file, search settings and database must be
		initialised.

	*/
	bool add_proof_task(size_t num_targets);

	/**
	\brief Create a new process which will generate conjectures
		using a Hidden Markov Model.

	\param N The number of conjectures to try generating.

	\pre Language, context file and database must be initialised.

	\returns True iff success.
	*/
	bool add_hmm_conjecture_task(size_t N);

	/**
	\brief Create a new process which will train the HMM conjecturer
		model on existing proven theorems.

	\param epochs The number of passes to perform over the dataset.

	\param dataset_size The limit on the number of statements to load
		from the database to train on.

	\pre Language, context file and database must be initialised.

	\returns True iff success.
	*/
	bool add_hmm_conj_train_task(size_t epochs, size_t dataset_size);

	/**
	\brief Create a new process which will create a new HMM
		conjecturer model.

	\param num_hidden The number of hidden states the new model
		should have.

	\param model_id The ID of the new model.

	\pre Language, context file and database must be initialised.

	\returns True iff success.
	*/
	bool create_hmm_conjecturer(size_t num_hidden, size_t model_id);

	/**
	\brief Run all proofs that have been set.
	*/
	void run();

private:
	/**
	\brief If any axioms are not stored as theorems in the database,
		add them.

	\details It is assumed in the proof processes that this is the
		case, and this seems like the best place to fix it if it's
		not the case.

	\pre m_db != nullptr && m_ctx != nullptr
	*/
	void check_axioms_in_db();

private:
	const size_t m_num_threads;
	atp::logic::LanguagePtr m_lang;
	atp::logic::ModelContextPtr m_ctx;
	size_t m_ctx_id, m_ss_id;  // ctx and search IDs from database
	atp::db::DatabasePtr m_db;
	atp::search::SearchSettings m_search_settings;
	atp::core::ProcessManager m_proc_mgr;
};


