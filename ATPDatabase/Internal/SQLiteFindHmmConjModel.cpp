/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteFindHmmConjModel.h"


namespace atp
{
namespace db
{


std::string SQLiteFindHmmConjModel::build() const
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	// m_model_id is optional

	std::stringstream query_builder;

	query_builder << "SELECT id, num_states, free_q FROM "
		"hmm_conjecturer_models WHERE ctx_id = "
		<< m_ctx_id;

	if (m_model_id.has_value())
	{
		query_builder << " AND id = " << *m_model_id;
	}

	// ensure that, if there are many models possible and the user
	// didn't specify one, we just pick arbitrarily (so, just pick
	// the top one).
	query_builder << " LIMIT 1";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


