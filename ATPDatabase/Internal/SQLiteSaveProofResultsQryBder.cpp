/**
\file

\author Samuel Barrett

*/


#include "SQLiteSaveProofResultsQryBder.h"
#include <sstream>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>


namespace atp
{
namespace db
{


std::string SQLiteSaveProofResultsQryBder::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_size.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	ATP_DATABASE_PRECOND(m_targets.has_value());
	// optional: m_proof_states, m_times, m_max_mems, m_num_exps,
	// m_helpers, m_ss_id
	ATP_DATABASE_PRECOND(m_proof_states.has_value()
		== m_ss_id.has_value());
	ATP_DATABASE_PRECOND(m_proof_states.has_value()
		== m_times.has_value());
	ATP_DATABASE_PRECOND(m_proof_states.has_value()
		== m_max_mems.has_value());
	ATP_DATABASE_PRECOND(m_proof_states.has_value()
		== m_num_exps.has_value());
	ATP_DATABASE_PRECOND(!m_helpers.has_value() ||
		m_proof_states.has_value());

	if (m_query_templates.find("insert_thm")
		== m_query_templates.end() ||
		m_query_templates.find("insert_proof_attempt")
		== m_query_templates.end() ||
		m_query_templates.find("insert_proof")
		== m_query_templates.end() ||
		m_query_templates.find("insert_usage")
		== m_query_templates.end())
	{
		ATP_DATABASE_LOG(error) << "Cannot find query templates "
			"for \"insert_thm\", \"insert_proof_attempt\""
			", \"insert_proof\" or \"insert_usage\""
			", please add these to the DB config file.";

		return "- ; -- bad query because error earlier.";
	}
	else
	{
		const std::string ctx_id_str =
			boost::lexical_cast<std::string>(*m_ctx_id);

		std::stringstream query_builder;
		std::string query;
		for (size_t i = 0; i < *m_size; i++)
		{
			// add the theorem to the database if it's not in there
			// already
			query = m_query_templates.at("insert_thm");

			const std::string stmt_str = '"' +
				(*m_targets)->at(i).to_str() + '"';

			boost::algorithm::replace_all(query,
				":stmt", stmt_str);
			boost::algorithm::replace_all(query,
				":ctx_id", ctx_id_str);

			query_builder << query << ";\n\n";

			// skip the rest if we are only loading theorems, not proof
			// attempts
			if (!m_proof_states.has_value())
				continue;

			const bool successful = ((*m_proof_states)[i] != nullptr &&
				(*m_proof_states)[i]->completion_state() ==
				atp::logic::ProofCompletionState::PROVEN);

			// add the theorem to the database if it's not in there
			// already
			query = m_query_templates.at("insert_proof_attempt");

			boost::algorithm::replace_all(query,
				":stmt", stmt_str);
			boost::algorithm::replace_all(query,
				":ctx_id", ctx_id_str);
			boost::algorithm::replace_all(query,
				":ss_id",
				boost::lexical_cast<std::string>(*m_ss_id));
			boost::algorithm::replace_all(query,
				":time_cost",
				boost::lexical_cast<std::string>((*m_times)[i]));
			boost::algorithm::replace_all(query,
				":max_mem",
				boost::lexical_cast<std::string>((*m_max_mems)[i]));
			boost::algorithm::replace_all(query,
				":num_expansions",
				boost::lexical_cast<std::string>((*m_num_exps)[i]));
			boost::algorithm::replace_all(query,
				":success", (successful ? "1" : "0"));

			query_builder << query << ";\n\n";

			// If the proof was successful, add the proof to the database
			// (obviously we do not want to do this if it failed)
			if (successful)
			{
				query = m_query_templates.at("insert_proof");

				boost::algorithm::replace_all(query,
					":stmt", stmt_str);
				boost::algorithm::replace_all(query,
					":ctx_id", ctx_id_str);
				boost::algorithm::replace_all(query,
					":proof", '"' +
					(*m_proof_states)[i]->to_str() + '"');

				query_builder << query << ";\n\n";

				// add theorem usages:
				if (m_helpers.has_value())
				{
					const auto& pf_state = m_proof_states->at(i);
					auto usages = pf_state->get_usage(*m_helpers);

					for (size_t j = 0; j < usages.size(); ++j)
					{
						// cannot add entries with zero count!
						if (usages[j] > 0)
						{
							query = m_query_templates.at(
								"insert_usage");

							const std::string helper_str = '"' +
								(*m_helpers)->at(j).to_str() + '"';

							boost::algorithm::replace_all(query,
								":stmt", stmt_str);
							boost::algorithm::replace_all(query,
								":ctx_id", ctx_id_str);
							boost::algorithm::replace_all(query,
								":helper_stmt", helper_str);
							boost::algorithm::replace_all(query,
								":uses",
								boost::lexical_cast<std::string>(
									usages[j]));

							query_builder << query << ";\n\n";
						}
					}
				}
			}
		}

		return query_builder.str();
	}
}


}  // namespace db
}  // namespace atp


