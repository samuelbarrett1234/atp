/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteSaveProofResultsQryBder.h"


namespace atp
{
namespace db
{


std::string SQLiteSaveProofResultsQryBder::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ss_id.has_value());
	ATP_DATABASE_PRECOND(m_size.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	ATP_DATABASE_PRECOND(m_targets.has_value());
	ATP_DATABASE_PRECOND(m_proof_states.has_value());
	ATP_DATABASE_PRECOND(m_times.has_value());
	ATP_DATABASE_PRECOND(m_max_mems.has_value());
	ATP_DATABASE_PRECOND(m_num_exps.has_value());
	// don't check m_helpers, that is optional.

	std::stringstream query_builder;
	query_builder << "BEGIN TRANSACTION;\n\n";

	for (size_t i = 0; i < *m_size; i++)
	{
		// add the theorem to the database if it's not in there
		// already
		query_builder << "INSERT INTO theorems (stmt, "
			<< "ctx) SELECT '" << (*m_targets)->at(i).to_str()
			<< "', " << *m_ctx_id << " WHERE NOT EXISTS("
			<< "SELECT 1 FROM theorems WHERE stmt == '"
			<< (*m_targets)->at(i).to_str() << "');\n\n";

		// a query for finding the theorem ID of the target statement
		const std::string find_thm_id =
			"(SELECT id FROM theorems WHERE stmt = '" +
			(*m_targets)->at(i).to_str() + "')";

		// add proof attempt to database
		query_builder << "INSERT INTO proof_attempts(thm_id, ss_id, "
			<< "time_cost, max_mem, num_expansions) VALUES ("
			<< find_thm_id << ", " << *m_ss_id << ", "
			<< (*m_times)[i] << ", " << (*m_max_mems)[i]
			<< ", " << (*m_num_exps)[i] << ");\n\n";

		// IF THE PROOF WAS SUCCESSFUL, add the proof to the database
		// (obviously we do not want to do this if it failed)
		if ((*m_proof_states)[i] != nullptr &&
			(*m_proof_states)[i]->completion_state() ==
			atp::logic::ProofCompletionState::PROVEN)
		{
			// make sure to not try inserting a new proof if there
			// is already a proof in the database!
			query_builder << "INSERT INTO proofs (thm_id, proof) "
				<< "SELECT " << find_thm_id << " , '"
				<< (*m_proof_states)[i]->to_str() << "' WHERE NOT EXISTS"
				<< " (SELECT 1 FROM proofs JOIN theorems ON "
				<< "thm_id=id WHERE stmt = '"
				<< (*m_targets)->at(i).to_str() << "');\n\n";

			// add theorem usages:
			if (m_helpers.has_value())
			{
				const auto& pf_state = m_proof_states->at(i);
				auto usages = pf_state->get_usage(*m_helpers);

				for (size_t i = 0; i < usages.size(); ++i)
				{
					// a query for finding the theorem ID of the
					// helper statement
					const std::string find_helper_thm_id =
						"(SELECT id FROM theorems WHERE stmt = '" +
						(*m_helpers)->at(i).to_str() + "')";

					// add it:
					query_builder << "INSERT INTO theorem_usage "
						<< "(target_thm_id, used_thm_id, cnt) VALUES"
						<< "( " << find_thm_id << ", "
						<< find_helper_thm_id << ", " << usages[i]
						<< ");\n\n";
				}
			}
		}
	}

	query_builder << "COMMIT;";

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


