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
	GET_HMM_CONJ_OBS_PARAMS
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


}  // namespace db
}  // namespace atp


