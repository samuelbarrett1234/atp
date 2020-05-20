#pragma once


/**
\file

\author Samuel Barrett

\brief Query builders are objects which create SQL statements
	for specific operations common to ATPDatabase users, and ensure
	the SQL syntax is correct for the particular SQL version and
	database implementation being used.

\details The main necessity of these objects is to (i) reduce the
	amount of code needed for query string writing in the IDatabase
	implementations, and (ii) have a uniform interface for creating
	these queries regardless of the difference in SQL semantics
	across DBMSs.

*/


#include <string>
#include <memory>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


/**
\brief An enumeration of common query types, for which many have
	corresponding extending interfaces.

*/
enum class QueryBuilderType
{
	// randomly select a number of theorems from the database
	RANDOM_THM_SELECTION,

	// save a set of theorems, possibly with proofs and proof
	// attempts, to the database
	SAVE_THMS_AND_PROOFS,

	// check that all the axioms are appropriately loaded into the DB
	CHECK_AXIOMS_IN_DB,

	// search for a HMM conjecture model in the database
	FIND_HMM_CONJ_MODEL,

	// given a reference to a HMM conjecture model, find all of its
	// state transition parameters
	GET_HMM_CONJ_ST_TRANS_PARAMS,

	// given a reference to a HMM conjecture model, find all of its
	// observation parameters.
	GET_HMM_CONJ_OBS_PARAMS,

	// save HMM conjecture model parameters
	SAVE_HMM_CONJ_PARAMS,

	// select a model context, based on the usage statistics of all
	// the contexts in the database
	SELECT_CTX,

	// select search settings, based on the selected model context,
	// and the effectiveness statistics of each search settings type
	// in each context
	SELECT_SS
};


/**
\brief An object for producing queries
*/
class ATP_DATABASE_API IQueryBuilder
{
public:
	virtual ~IQueryBuilder() = default;

	/**
	\brief Create a the query that you have been building.

	\pre Check that you have filled in all the compulsory information
		before calling this!
	*/
	virtual std::string build() = 0;
};
typedef std::unique_ptr<IQueryBuilder> QueryBuilderPtr;


/**
\brief Interface for building a query to select theorems from the
	database.

\see RANDOM_THM_SELECTION

\details This query is only ready to build when the limit and context
	have been set. By default, it only returns theorems with proofs,
	however you can set it so that it only returns unproven theorems,
	instead, if you wish.
*/
class ATP_DATABASE_API IRndThmSelectQryBder :
	public IQueryBuilder
{
public:
	virtual ~IRndThmSelectQryBder() = default;

	/**
	\brief Set the maximum number of theorems for the query to return

	\returns this
	*/
	virtual IRndThmSelectQryBder* set_limit(size_t N) = 0;

	/**
	\brief Set the model context info.

	\returns this
	*/
	virtual IRndThmSelectQryBder* set_context(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;

	/*
	\brief Select from proven theorems from the database, or unproven
		theorems?

	\param proven If true, will only select from proven theorems. If
		false, will only select from UNproven theorems.
	*/
	virtual IRndThmSelectQryBder* set_proven(bool proven) = 0;

	/**
	\brief Restore this object's state to the same as it was when it
		was created.

	\returns this
	*/
	virtual IRndThmSelectQryBder* reset() = 0;
};


/**
\brief Interface for saving a collection of proof attempt results.

\details This can be used to save theorems, theorems and proof
	attempts, or all of: theorems, proof attempts, and proofs, and
	theorem usages in the proofs.

\see SAVE_THMS_AND_PROOFS
*/
class ATP_DATABASE_API ISaveProofResultsQryBder :
	public IQueryBuilder
{
public:
	virtual ~ISaveProofResultsQryBder() = default;

	/**
	\brief Set the model context info.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* set_context(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;

	/**
	\brief Set the search settings ID from the database.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* set_search_settings(
		size_t ss_id) = 0;

	/**
	\brief Restore this object's state to the same as it was when it
		was created.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* reset() = 0;

	/**
	\brief Set the theorems which were targetted by each proof.

	\pre p_targets != nullptr

	\pre All of these arrays must be the same size.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_target_thms(
		const logic::StatementArrayPtr& p_targets) = 0;

	/**
	\brief Set which theorems were loaded from the database in order
		to assist finding proofs, if any. This is optional.

	\note This function is optional, and is not needed for building
		the query.

	\details This will have been exactly the set of theorems you
		added to the knowledge kernel in order to help generate more
		successors. These are needed so that we can determine their
		usage in the proof, to keep track of how "useful" every
		theorem is.

	\pre p_helpers != nullptr

	\note Of course, this array doesn't have to be the same size as
		all the others.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_helper_thms(
		const logic::StatementArrayPtr& p_helpers) = 0;

	/**
	\brief Add the proof states of those which were successfully
		proven.

	\details If any element of this array is nullptr, it will be
		assumed that the corresponding theorem was not successfully
		proven, and it will not be added to the `proofs` table.

	\pre All of these arrays must be the same size.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_proof_states(
		const std::vector<logic::ProofStatePtr>& proof_states) = 0;

	/**
	\brief Set the time cost for each proof attempt

	\pre All of these arrays must be the same size.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_proof_times(
		const std::vector<float>& proof_times) = 0;

	/**
	\brief Set the maximum memory usage of each proof attempt

	\pre All of these arrays must be the same size.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_max_mem_usages(
		const std::vector<size_t>& max_mem_usages) = 0;

	/**
	\brief Set the number of node expansions in each proof attempt

	\pre All of these arrays must be the same size.

	\returns this
	*/
	virtual ISaveProofResultsQryBder* add_num_node_expansions(
		const std::vector<size_t>& num_node_expansions) = 0;
};


/**
\brief Interface for making sure all of the axioms of a given context
	are appropriately set in the database before doing any proving.

\see CHECK_AXIOMS_IN_DB
*/
class ATP_DATABASE_API ICheckAxInDbQryBder :
	public IQueryBuilder
{
public:
	virtual ~ICheckAxInDbQryBder() = default;

	/**
	\brief Set the context of this theorem

	\returns this
	*/
	virtual ICheckAxInDbQryBder* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;

	/**
	\brief Set the language associated with the given context

	\returns this
	*/
	virtual ICheckAxInDbQryBder* set_lang(
		const logic::LanguagePtr& p_lang) = 0;

	/**
	\brief Restore this object's state to the same as it was when it
		was created.

	\returns this
	*/
	virtual ICheckAxInDbQryBder* reset() = 0;
};


/**
\brief Interface for getting basic information about a HMM conjecture
	model.

\see FIND_HMM_CONJ_MODEL
*/
class ATP_DATABASE_API IFindHmmConjModel :
	public IQueryBuilder
{
public:
	virtual ~IFindHmmConjModel() = default;

	/**
	\brief Restore object to the state it was when it was constructed

	\returns this
	*/
	virtual IFindHmmConjModel* reset() = 0;

	/**
	\brief Optionally, provide a model ID to search for.

	\returns this
	*/
	virtual IFindHmmConjModel* set_model_id(size_t mid) = 0;

	/**
	\brief Set the model context

	\returns this
	*/
	virtual IFindHmmConjModel* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;
};


/**
\brief Interface for getting the parameters of a HMM
	conjecture model (both state transition parameters, and
	observation parameters).

\see GET_HMM_CONJ_ST_TRANS_PARAMS

\see GET_HMM_CONJ_OBS_PARAMS
*/
class ATP_DATABASE_API IGetHmmConjectureModelParams :
	public IQueryBuilder
{
public:
	virtual ~IGetHmmConjectureModelParams() = default;

	/**
	\brief Restore object to the state it was when it was constructed

	\returns this
	*/
	virtual IGetHmmConjectureModelParams* reset() = 0;

	/**
	\brief Set the model ID to find the parameters for.

	\returns this
	*/
	virtual IGetHmmConjectureModelParams* set_model_id(size_t mid) = 0;

	/**
	\brief Set the model context

	\returns this
	*/
	virtual IGetHmmConjectureModelParams* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;
};


/**
\brief This query builder is for putting together a query to save
	model parameters for the HMM conjecturer model.

\details The model context information and model ID are compulsory,
	but everything else is optional.
*/
class ATP_DATABASE_API ISaveHmmConjModelParams :
	public IQueryBuilder
{
public:
	virtual ~ISaveHmmConjModelParams() = default;

	/**
	\brief Reset the parameters as they were when this object was
		constructed.

	\returns this
	*/
	virtual ISaveHmmConjModelParams* reset() = 0;

	/**
	\brief Set the model ID to find the parameters for.

	\returns this
	*/
	virtual ISaveHmmConjModelParams* set_model_id(size_t mid) = 0;

	/**
	\brief Set the model context

	\returns this
	*/
	virtual ISaveHmmConjModelParams* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;

	/**
	\brief Set the number of hidden states in this model

	\returns this
	*/
	virtual ISaveHmmConjModelParams* set_num_states(size_t n) = 0;

	/**
	\brief Set the "q" parameter for the geometric distribution
		modelling the free variable IDs

	\returns this
	*/
	virtual ISaveHmmConjModelParams* set_free_q(float q) = 0;

	/**
	\brief Add a state transition from "pre" to "post" which occurs
		which probability "prob"

	\returns this
	*/
	virtual ISaveHmmConjModelParams* add_state_trans(size_t pre,
		size_t post, float prob) = 0;

	/**
	\brief State that the model observes the symbol with ID "symb_id"
		in state "state" with probability "prob"

	\returns this
	*/
	virtual ISaveHmmConjModelParams* add_observation(size_t state,
		size_t symb_id, float prob) = 0;
};


/**
\brief A query which randomly selects a model context, according to
	existing usage statistics as well.

\details Intended to be good for automatically selecting contexts to
	prove statements in. Returns exactly one row with columns
	(context filename, context ID).
*/
class ATP_DATABASE_API ISelectModelContext :
	public IQueryBuilder
{
public:
	virtual ~ISelectModelContext() = default;

	// no extra functions needed
};


/**
\brief A query which randomly selects search settings, according to
	existing usage statistics and taking into account the selected
	context.

\details Intended to be good for automatically selecting settings to
	prove statements with. Returns exactly one row with columns
	(search settings filename, search settings ID).
*/
class ATP_DATABASE_API ISelectSearchSettings :
	public IQueryBuilder
{
public:
	virtual ~ISelectSearchSettings() = default;

	virtual ISelectSearchSettings* set_ctx_id(size_t ctx_id) = 0;
};


}  // namespace db
}  // namespace atp


