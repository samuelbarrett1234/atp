/**
\file

\author Samuel Barrett

*/


#include "SQLiteSelectSearchSettings.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>


namespace atp
{
namespace db
{


std::string SQLiteSelectSearchSettings::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());

	if (m_query_templates.find("select_search_settings")
		== m_query_templates.end())
	{
		ATP_DATABASE_LOG(error) << "Cannot find query templates "
			"for \"select_search_settings\""
			", please add these to the DB config file.";

		return "- ; -- bad query because error earlier.";
	}
	else
	{
		std::string query = m_query_templates.at(
			"select_search_settings");

		boost::algorithm::replace_all(
			query,
			":ctx_id", boost::lexical_cast<std::string>(*m_ctx_id));

		return query;
	}
}


}  // namespace db
}  // namespace atp


