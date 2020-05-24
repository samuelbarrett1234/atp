/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteSelectModelContext.h"


namespace atp
{
namespace db
{


std::string SQLiteSelectModelContext::build()
{
	if (m_query_templates.find("select_model_context")
		== m_query_templates.end())
	{
		ATP_DATABASE_LOG(error) << "Cannot find query templates "
			"for \"select_model_context\""
			", please add these to the DB config file.";

		return "- ; -- bad query because error earlier.";
	}
	else
	{
		return m_query_templates.at("select_model_context");
	}
}


}  // namespace db
}  // namespace atp


