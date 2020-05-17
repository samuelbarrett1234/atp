#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an interface for selecting theorems from the database
	in a clever manner to help prove a particular set of theorems.

*/


#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/**
\brief A selection strategy for picking theorems from the database to
	help find a proof for a set of target theorems.

\details These strategies work in stages: (i) first you use
	`set_targets` to tell it what you're trying to prove, then (ii)
	you use `create_getter_query` to let the strategy provide a query
	which will find the theorems, then (iii) use `load_values` while
	executing the query whenever the query has values to provide,
	which will allow the strategy to extract information from the
	query, and finally (iv) call `done` when the query exits to
	obtain an array of all the helper theorems.
*/
class ATP_SEARCH_API ISelectionStrategy
{
public:
	virtual ~ISelectionStrategy() = default;

	/**
	\brief Set the target theorems that we will get helper theorems
		for.

	\pre p_targets != nullptr
	*/
	virtual void set_targets(
		const logic::StatementArrayPtr& p_targets) = 0;

	/**
	\brief Get the query builder for finding the theorems from the
		database.

	\pre p_db != nullptr && `set_targets` has been called more
		recently than `done`.

	\returns The query builder to allow you to create the query which
		will find the helper theorems from the database.

	\post The produced transaction will be an IQueryTransaction
	*/
	virtual db::QueryBuilderPtr create_getter_query(
		const db::DatabasePtr& p_db) = 0;

	/**
	\brief Load values from the query (this should be called at each
		step.)

	\pre query.has_values() && `create_getter_query` has been called
		more recently than `done`.
	*/
	virtual void load_values(
		const db::IQueryTransaction& query) = 0;

	/**
	\brief Call this when the query finishes, to compute which
		theorems were best, and return them.

	\pre The query (provided earlier) has finished.

	\returns The array of helper theorems.

	\post This resets the current targets, i.e. resets the object to
		the state it was when it was initialised.
	*/
	virtual logic::StatementArrayPtr done() = 0;
};


}  // namespace search
}  // namespace atp


