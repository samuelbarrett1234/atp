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
	RANDOM_PROVEN_THM_SELECTION,

	// save a set of theorems, possibly with proofs and proof
	// attempts, to the database
	SAVE_THMS_AND_PROOFS,

	// ensure that a particular theorem exists in the database
	INSERT_THM_IF_NOT_EXISTS
};


/**
\brief An object for producing queries
*/
class ATP_DATABASE_API IQueryBuilder
{
public:
	virtual ~IQueryBuilder() = default;

	/**
	\brief Attempt to create a transaction
	*/
	virtual std::string build() = 0;
};
typedef std::unique_ptr<IQueryBuilder> QueryBuilderPtr;


/**
\brief Interface for setting data for the Random Proven Theorem
	Select Query Builder

\details This query is only ready to build when the limit and context
	have been set.
*/
class ATP_DATABASE_API IRndProvenThmSelectQryBder :
	public IQueryBuilder
{
public:
	virtual ~IRndProvenThmSelectQryBder() = default;

	/**
	\brief Set the maximum number of theorems for the query to return

	\returns this
	*/
	virtual IRndProvenThmSelectQryBder* set_limit(size_t N) = 0;

	/**
	\brief Set the model context info.

	\returns this
	*/
	virtual IRndProvenThmSelectQryBder* set_context(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) = 0;

	/**
	\brief Restore this object's state to the same as it was when it
		was created.

	\returns this
	*/
	virtual IRndProvenThmSelectQryBder* reset() = 0;
};


/**
\brief Interface for saving a collection of proof attempt results.
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
\brief Interface for inserting a theorem into the database (if it
	doesn't already exist).
*/
class ATP_DATABASE_API IInsertThmIfNotExQryBder :
	public IQueryBuilder
{
public:
	virtual ~IInsertThmIfNotExQryBder() = default;

	/**
	\brief Set the text of the theorem to check

	\returns this
	*/
	virtual IInsertThmIfNotExQryBder* set_thm(
		const std::string& thm) = 0;

	/**
	\brief Set the context of this theorem

	\returns this
	*/
	virtual IInsertThmIfNotExQryBder* set_ctx(
		size_t ctx_id) = 0;

	/**
	\brief Restore this object's state to the same as it was when it
		was created.

	\returns this
	*/
	virtual IInsertThmIfNotExQryBder* reset() = 0;
};


}  // namespace db
}  // namespace atp


