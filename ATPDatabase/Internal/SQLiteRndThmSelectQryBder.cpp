/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteRndThmSelectQryBder.h"


namespace atp
{
namespace db
{


std::string SQLiteRndThmSelectQryBder::build()
{
	ATP_DATABASE_PRECOND(m_limit.has_value());
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());

	std::stringstream query_builder;

	// IMPORTANT: only load theorems for which there exist proofs!
	query_builder << "SELECT stmt FROM theorems WHERE ctx = "
		<< *m_ctx_id << " AND ";
	
	if (!m_find_proven)
		query_builder << "NOT ";

	query_builder << " EXISTS (SELECT 1 FROM proofs WHERE thm_id=id";

	// only do this when looking for proven theorems, because of
	// course this does not have the intended effect when trying to
	// load unproven theorems
	// (axioms will always have a proof, anyway)
	if (m_find_proven)
		query_builder << " AND is_axiom = 0";  // don't load axioms!

	query_builder << " ) ORDER BY RANDOM() LIMIT " << *m_limit << ";";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


