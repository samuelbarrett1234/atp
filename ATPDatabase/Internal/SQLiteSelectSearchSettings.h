#pragma once


/**
\file

\author Samuel Barrett

\brief Implementation of ISelectSearchSettings for SQLite

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
\brief Implementation of a search settings select for SQLite
	databases.
*/
class ATP_DATABASE_API SQLiteSelectSearchSettings :
	public ISelectSearchSettings
{
public:
	SQLiteSelectSearchSettings(
		const std::map<std::string, std::string>& query_templates) :
		m_query_templates(query_templates)
	{ }

	std::string build() override;
	inline ISelectSearchSettings* set_ctx_id(size_t ctx_id) override
	{
		m_ctx_id = ctx_id;
		return this;
	}

private:
	std::map<std::string, std::string> m_query_templates;

	boost::optional<size_t> m_ctx_id;
};


}  // namespace db
}  // namespace atp


