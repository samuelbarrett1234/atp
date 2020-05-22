/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteSelectSearchSettings.h"


namespace atp
{
namespace db
{


std::string SQLiteSelectSearchSettings::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());

	std::stringstream query_builder;

	// order by probability of success

	query_builder << "SELECT filename, ss_id FROM search_settings "
		"LEFT OUTER NATURAL JOIN (SELECT ss_id, "
		"(IFNULL(SUM(success),0) + 1.0) / (COUNT(success) + 2.0) AS p_success "
		"FROM (proof_attempts NATURAL JOIN theorems) "
		"WHERE ctx_id = " << *m_ctx_id <<
		" GROUP BY ss_id) WHERE serve = 1 "
		// interesting part: add random noise to avg_cost
		"ORDER BY p_success + RANDOM() * 2.0e-20 DESC LIMIT 1;";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


