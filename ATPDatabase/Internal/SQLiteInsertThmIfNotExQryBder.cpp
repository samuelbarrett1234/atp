/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteInsertThmIfNotExQryBder.h"


namespace atp
{
namespace db
{


std::string SQLiteInsertThmIfNotExQryBder::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_thm.has_value());

	std::stringstream query_builder;

	query_builder << "INSERT OR IGNORE INTO theorems(stmt, ctx) "
		<< "VALUES ( '" << *m_thm << "', " << *m_ctx_id << ");";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


