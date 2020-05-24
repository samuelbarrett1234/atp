/**
\file

\author Samuel Barrett

*/


#include "SQLiteRndThmSelectQryBder.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>


namespace atp
{
namespace db
{


std::string SQLiteRndThmSelectQryBder::build()
{
	ATP_DATABASE_PRECOND(m_limit.has_value());
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());

	if (m_query_templates.find("random_proven_thms")
		== m_query_templates.end() ||
		m_query_templates.find("random_unproven_thms")
		== m_query_templates.end())
	{
		ATP_DATABASE_LOG(error) << "Cannot find query templates "
			"for \"random_proven_thms\" or \"random_unproven_thms\""
			", please add these to the DB config file.";

		return "- ; -- bad query because error earlier.";
	}
	else
	{
		std::string query = m_find_proven ?
			m_query_templates.at("random_proven_thms") :
			m_query_templates.at("random_unproven_thms");

		boost::algorithm::replace_all(
			query,
			":ctx_id", boost::lexical_cast<std::string>(*m_ctx_id));

		boost::algorithm::replace_all(
			query,
			":limit", boost::lexical_cast<std::string>(*m_limit));

		return query;
	}
}


}  // namespace db
}  // namespace atp


