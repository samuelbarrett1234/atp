/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteGetHmmConjStTransParams.h"


namespace atp
{
namespace db
{


std::string SQLiteGetHmmConjStTransParams::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	ATP_DATABASE_PRECOND(m_model_id.has_value());

	std::stringstream query_builder;

	query_builder << "SELECT pre_state, post_state, prob "
		"FROM hmm_conjecturer_state_transitions WHERE "
		"id = " << *m_model_id << ";";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


