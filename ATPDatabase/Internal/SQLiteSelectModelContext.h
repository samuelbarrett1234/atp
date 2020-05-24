#pragma once


/**
\file

\author Samuel Barrett

\brief Implementation of ISelectModelContext for SQLite

*/


#include <map>
#include <string>
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
	SQLiteSelectModelContext(
		const std::map<std::string, std::string>& query_templates) :
		m_query_templates(query_templates)
	{ }

	std::string build() override;

private:
	std::map<std::string, std::string> m_query_templates;
};


}  // namespace db
}  // namespace atp


