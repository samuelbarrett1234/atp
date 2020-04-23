#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the core Database object interface of this library.

*/


#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "DBContainers.h"
#include "DBOperations.h"
#include "ILockManager.h"


namespace atp
{
namespace db
{


class IOpStarter;  // forward declaration
typedef std::shared_ptr<IOpStarter> OpStarterPtr;


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
	virtual ILockManager& lock_mgr() = 0;
	virtual logic::LangType logic_lang() const = 0;

	/**
	\brief Try to begin an operation.

	\param op_str The operation in string format (there are specific
		formats for this).

	\returns Nullptr if the operation is invalid, otherwise returns a
		OpStarterPtr which is an object that can produce the DB
		operation once it has acquired a lock.
	*/
	virtual OpStarterPtr begin_operation(const std::string& op_str) const = 0;
};


/**
\details Class for constructing operations when their locks are
	obtained.

\detauls Since "operations are hard to start" as they require the
	correct locks, this is an object used for keeping the info
	needed to construct a particular operation, and knowing what
	resources are needed, but only constructing the object when the
	lock has been attained.

\note If you wish, this object can construct many copies of the
	original operation, however most likely you will just want to
	dispose of this object when you obtain the lock and call
	`lock_obtained`.
*/
class ATP_DATABASE_API IOpStarter
{
public:
	virtual ~IOpStarter() = default;

	/**
	\brief Get all the resources that this operation needs a lock on
		(before constructing such an object).
	*/
	virtual const ResourceList& res_needed() const = 0;

	/**
	\brief Once a lock has been created **for this instance**, create
		the DB operation object.

	\warning Do not reuse locks between calls to this function - each
		lock should be uniquely created for each call to
		`lock_obtained`.

	\returns A new DB operation object, which assumes control of the
		lock, and has no ties to this object.
	*/
	virtual DBOpPtr lock_obtained(LockPtr&& p_lock) = 0;
};


}  // namespace db
}  // namespace atp


