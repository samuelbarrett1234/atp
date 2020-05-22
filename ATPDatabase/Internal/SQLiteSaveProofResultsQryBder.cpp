/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include <boost/lexical_cast.hpp>
#include "SQLiteSaveProofResultsQryBder.h"


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

	std::stringstream query_builder;

	for (size_t i = 0; i < *m_size; i++)
	{
		// add the theorem to the database if it's not in there
		// already
		query_builder << "INSERT OR IGNORE INTO theorems (stmt, "
			<< "ctx_id, induced_priority) VALUES ('" <<
			(*m_targets)->at(i).to_str() << "', " << *m_ctx_id
			<< ", 0.0);\n\n";

		// skip the rest if we are only loading theorems, not proof
		// attempts
		if (!m_proof_states.has_value())
			continue;

		// a common table expression for finding the theorem ID of the
		// target statement
		// WARNING: theorem statements by themselves are not unique!
		// It is only unique when paired with a context!
		const std::string thm_id_with_expr =
			"WITH my_thm(my_id) AS (SELECT thm_id FROM theorems WHERE "
			"stmt = '" + (*m_targets)->at(i).to_str() +
			"' AND ctx_id = " + boost::lexical_cast<std::string>(
				*m_ctx_id) + ")\n";

		const bool successful = ((*m_proof_states)[i] != nullptr &&
			(*m_proof_states)[i]->completion_state() ==
			atp::logic::ProofCompletionState::PROVEN);

		// add proof attempt to database
		query_builder << thm_id_with_expr;
		query_builder << "INSERT INTO proof_attempts(thm_id, ss_id, "
			"time_cost, max_mem, num_expansions, success) VALUES ((SELECT "
			"my_id FROM my_thm), "
			<< *m_ss_id << ", " << (*m_times)[i] << ", "
			<< (*m_max_mems)[i] << ", " << (*m_num_exps)[i] << ", "
			// only set successful bit to 1 if the theorem has not been
			// proven before
			"CASE WHEN EXISTS (SELECT 1 FROM proofs JOIN my_thm ON "
			"thm_id=my_id) THEN 0 ELSE "
			<< (successful ? 1 : 0) << " END);\n\n";

		// IF THE PROOF WAS SUCCESSFUL, add the proof to the database
		// (obviously we do not want to do this if it failed)
		if (successful)
		{
			// make sure to not try inserting a new proof if there
			// is already a proof in the database!
			// WARNING: this theorem might already have a proof in
			// the database - don't add it again!
			query_builder << thm_id_with_expr;
			query_builder << "INSERT INTO proofs "
				"(thm_id, proof) SELECT (SELECT my_id FROM my_thm), "
				"'" << (*m_proof_states)[i]->to_str()
				<< "' WHERE NOT EXISTS (SELECT 1 FROM proofs WHERE"
				" thm_id=(SELECT my_id FROM my_thm));\n\n";

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
						// a common table expression for finding the
						// theorem ID of the helper statement (this
						// appears after the first WITH cte)
						// WARNING: theorem statements by themselves
						// are not unique! It is only unique when
						// paired with a context!
						const std::string helper_thm_id_with_expr =
							", helper_thm(helper_id) AS (SELECT "
							"thm_id FROM theorems WHERE stmt = '" +
							(*m_helpers)->at(j).to_str() +
							"' AND ctx_id = " +
							boost::lexical_cast<std::string>(
								*m_ctx_id) + ")\n";

						// add it:
						query_builder << thm_id_with_expr
							<< helper_thm_id_with_expr;
						query_builder << "INSERT OR IGNORE INTO "
							"theorem_usage (target_thm_id, "
							"used_thm_id, cnt) VALUES((SELECT my_id "
							"FROM my_thm), (SELECT helper_id FROM "
							"helper_thm), " << usages[j] << ");\n\n";
					}
				}
			}
		}
	}

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


