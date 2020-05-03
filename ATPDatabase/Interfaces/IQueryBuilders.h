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
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


/**
\brief An enumeration of common query types, for which many have
	corresponding extending interfaces.

*/
enum class CommQueryTypes
{
	// randomly select a number of theorems from the database
	RANDOM_PROVEN_THM_SELECTION,

	// save a set of theorems, possibly with proofs and proof
	// attempts, to the database
	SAVE_THMS_AND_PROOFS,

	// enumerate the (ID, Name, Filename) rows of the model contexts
	// table
	GET_MODEL_CONTEXTS_DATA,

	// enumerate the (ID, Name, Filename) rows of the search settings
	// table
	GET_SEARCH_SETTINGS_DATA
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





}  // namespace db
}  // namespace atp


