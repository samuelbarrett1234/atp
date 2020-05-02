#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the core Database object interface of this library.

*/


#include <memory>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"


namespace atp
{
namespace db
{


class ITransaction;  // forward declaration
typedef std::unique_ptr<ITransaction> TransactionPtr;


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
	virtual logic::LanguagePtr logic_lang() const = 0;

	/**
	\brief Begin executing a transaction.

	\returns A new transaction, or nullptr if the query failed to
		build.
	*/
	virtual TransactionPtr begin_transaction(
		const std::string& query_text) = 0;

	/**
	\brief Get model context filename, from corresponding name, if
		it exists in the database.
	*/
	virtual boost::optional<std::string> model_context_filename(
		const std::string& model_context_name) = 0;

};
typedef std::shared_ptr<IDatabase> DatabasePtr;


}  // namespace db
}  // namespace atp


