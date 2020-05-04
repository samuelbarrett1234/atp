/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteRndProvenThmSelectQryBder.h"


namespace atp
{
namespace db
{


std::string SQLiteRndProvenThmSelectQryBder::build()
{
	ATP_DATABASE_PRECOND(m_limit.has_value());
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());

	std::stringstream query_builder;

	// IMPORTANT: only load theorems for which there exist proofs!
	query_builder << "SELECT stmt FROM theorems JOIN proofs ON thm_id=id"
		" WHERE ctx = " << *m_ctx_id <<
		" AND is_axiom = 0" <<  // don't load axioms!
		" ORDER BY RANDOM() LIMIT " << *m_limit << ";";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


