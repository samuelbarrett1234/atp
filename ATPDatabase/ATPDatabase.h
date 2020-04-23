#pragma once


/*

\file

\author Samuel Barrett

\brief Main header file for this library.

*/


#include "ATPDatabaseAPI.h"
#include "Interfaces/Data.h"
#include "Interfaces/DBIterators.h"
#include "Interfaces/DBContainers.h"
#include "Interfaces/ILockManager.h"
#include "Interfaces/IBufferManager.h"
#include "Interfaces/DBOperations.h"
#include "Interfaces/IDatabase.h"


/**
\namespace atp::db

\brief The namespace containing all database-related functions and
	objects.
*/


namespace atp
{
namespace db
{


/**
\brief Create a new database from a JSON database schema file.

\param in_json_schema The input stream from which to read the schema.

\param out_json_db_config_file The output stream where it is safe
	to write the database's configuration file, to allow you to load
	the database again in the future. Should be a stream expecting to
	read human-readable text.

\returns A new database object if success, otherwise returns nullptr
	on failure.
*/
ATP_DATABASE_API DatabasePtr create_from_schema(
	std::istream& in_json_schema,
	std::ostream& out_db_config_file);


/**
\brief Load a database from a configuration file (which will have
	been written then the database was first created from its schema)

\see create_from_schema

\returns A new database object if success, otherwise returns nullptr
	on failure.
*/
ATP_DATABASE_API DatabasePtr load_from_config_file(
	std::istream& in_db_config_file);


/**
\brief Create an in-memory database from the given schema, which will
	lose all of its data when it gets destroyed.

\warning There is no supported way to easily save all the data once
	this database object is created, if you were to change your mind
	about it being a purely in-memory database.

\returns A new database object if success, otherwise returns nullptr
	on failure.
*/
ATP_DATABASE_API DatabasePtr create_in_mem_db_from_schema(
	std::istream& in_json_schema);


/**
\brief Helper function for converting from the database's array type
	to the logic's statement array type.

\pre d_arr.type() == STMT
*/
ATP_DATABASE_API logic::StatementArrayPtr db_arr_to_stmt_arr(
	const DArray& d_arr);


}  // namespace db
}  // namespace atp


