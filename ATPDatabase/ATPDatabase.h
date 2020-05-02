#pragma once


/*

\file

\author Samuel Barrett

\brief Main header file for this library.

*/


#include "ATPDatabaseAPI.h"
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
\brief Create a new database object from the given SQLite database
	filename.

\returns A valid database object if the SQL database construction was
	successful, or nullptr if the filename was incorrect / the file
	was invalid.
*/
ATP_DATABASE_API DatabasePtr load_from_file(
	const std::string& filename);


}  // namespace db
}  // namespace atp


