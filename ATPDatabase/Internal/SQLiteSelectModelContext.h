#pragma once


/**
\file

\author Samuel Barrett

\brief Implementation of ISelectModelContext for SQLite

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


/**
\brief Implementation of a model context select for SQLite
	databases.
*/
class ATP_DATABASE_API SQLiteSelectModelContext :
	public ISelectModelContext
{
public:
	std::string build() override;
};


}  // namespace db
}  // namespace atp


