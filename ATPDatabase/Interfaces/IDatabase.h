#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the core Database object interface of this library.

*/


#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


/**
\brief The core object of this library - basically represents a
	collection of tables.

\details The main purpose of this object is to help constructing
	IDBOperation objects, which allow you to query database state
	and update the database. Databases are intrinsically tied to
	a kind of logic, and all of their tables use only that logic
	type. (In other words, each database holds its own Language
	object).
*/
class ATP_DATABASE_API IDatabase
{
public:
	virtual ~IDatabase() = default;

	virtual std::string name() const = 0;
	virtual std::string description() const = 0;
	virtual logic::LangType logic_lang() const = 0;

	// database operations

	/**
	\brief Create a model context from the given model context name,
		using the database to find the right model context file.

	\returns Nullptr if either the name was invalid, or the file in
		the database was not found.
	*/
	virtual logic::ModelContextPtr create_model_context(
		const std::string& model_context_name) = 0;

};
typedef std::shared_ptr<IDatabase> DatabasePtr;


}  // namespace db
}  // namespace atp


