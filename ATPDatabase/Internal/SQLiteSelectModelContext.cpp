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
	return "SELECT filename, ctx_id FROM model_contexts "
		"NATURAL JOIN (SELECT ctx_id, SUM(time_cost) AS total_cost "
		"FROM proof_attempts NATURAL JOIN theorems GROUP BY ctx_id) "
		// interesting part: add random noise to total costs
		"ORDER BY total_cost + random() * 1.0e-16 ASC LIMIT 1";
}


}  // namespace db
}  // namespace atp


