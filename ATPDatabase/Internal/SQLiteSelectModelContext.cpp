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
		"JOIN (SELECT ctx, SUM(time_cost) AS total_cost FROM "
		"proof_attempts JOIN theorems ON thm_id=id GROUP BY ctx) "
		// interesting part: add random noise to total costs
		"ON ctx=ctx_id ORDER BY total_cost + random() * 1.0e-16 "
		"ASC LIMIT 1";
}


}  // namespace db
}  // namespace atp


