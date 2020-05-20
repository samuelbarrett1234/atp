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

	query_builder << "SELECT filename,ss_id FROM search_settings "
		"NATURAL JOIN (SELECT ss_id, "
		"AVG(time_cost) AS avg_cost "
		"FROM (proof_attempts JOIN theorems ON "
		"thm_id=id LEFT OUTER NATURAL JOIN proofs) WHERE ctx=" <<
		*m_ctx_id <<
		// interesting part: add random noise to p_success
		" GROUP BY ss_id) ORDER BY avg_cost + RANDOM() * 1.0e-16 "
		"ASC LIMIT 1;";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


