/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteGetHmmConjObsParams.h"


namespace atp
{
namespace db
{


std::string SQLiteGetHmmConjObsParams::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	ATP_DATABASE_PRECOND(m_model_id.has_value());

	std::stringstream query_builder;

	query_builder << "SELECT hidden_state, symb_id, prob "
		"FROM hmm_conjecturer_symbol_observations WHERE "
		"id = " << *m_model_id;

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


