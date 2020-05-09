/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteSaveHmmConjModelParams.h"


namespace atp
{
namespace db
{


std::string SQLiteSaveHmmConjModelParams::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_model_id.has_value());

	std::stringstream query_builder;

	if (m_num_states.has_value())
	{
		// check that the state transitions are between valid states
		ATP_DATABASE_PRECOND(std::all_of(m_st_trans.begin(),
			m_st_trans.end(), [this](const auto& tup) {
				return tup.get<0>() < *m_num_states &&
					tup.get<1>() < *m_num_states;
			}));

		// check that the observations are from valid states
		ATP_DATABASE_PRECOND(std::all_of(m_st_obs.begin(),
			m_st_obs.end(), [this](const auto& tup) {
				return tup.get<0>() < *m_num_states;
			}));
	}

	if (m_num_states.has_value() || m_free_q.has_value())
	{
		query_builder << "INSERT OR REPLACE INTO "
			"hmm_conjecturer_models (ctx_id, id, ";

		if (m_num_states.has_value())
		{
			query_builder << "num_states";
			if (m_free_q.has_value())
			{
				query_builder << ", free_q";
			}
		}
		else
		{
			query_builder << "free_q";
		}

		query_builder << ") VALUES (" << *m_ctx_id << ", "
			<< *m_model_id << ", ";

		if (m_num_states.has_value())
		{
			query_builder << *m_num_states;
			if (m_free_q.has_value())
			{
				query_builder << ", " << *m_free_q;
			}
		}
		else
		{
			query_builder << *m_free_q;
		}

		query_builder << ");\n\n";
	}

	for (const auto& tup : m_st_trans)
	{
		query_builder << "INSERT OR REPLACE INTO "
			"hmm_conjecturer_state_transitions (id, pre_state, "
			"post_state, prob) VALUES (" << *m_model_id << ", "
			<< tup.get<0>() << ", " << tup.get<1>() << ", "
			<< tup.get<2>() << ");\n\n";
	}

	for (const auto& tup : m_st_obs)
	{
		query_builder << "INSERT OR REPLACE INTO "
			"hmm_conjecturer_symbol_observations (id, hidden_state, "
			"symbol, prob) VALUES (" << *m_model_id << ", "
			<< tup.get<0>() << ", '" << tup.get<1>() << "', "
			<< tup.get<2>() << ");\n\n";
	}

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


