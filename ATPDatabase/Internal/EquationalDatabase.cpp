/**
\file

\author Samuel Barrett

*/


#include "EquationalDatabase.h"
#include <sstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include "SQLiteTransaction.h"


namespace atp
{
namespace db
{


DatabasePtr EquationalDatabase::load_from_file(const std::string& filename)
{
	if (!boost::filesystem::is_regular_file(filename))
		return nullptr;

	auto p_db = std::make_shared<EquationalDatabase>();
	int rc;

	rc = sqlite3_open(filename.c_str(), &p_db->m_db);
	if (rc != 0)
	{
		return nullptr;
	}
	ATP_DATABASE_ASSERT(p_db->m_db != nullptr);

	p_db->m_lang = logic::create_language(
		logic::LangType::EQUATIONAL_LOGIC);

	return p_db;
}


EquationalDatabase::~EquationalDatabase()
{
	sqlite3_close(m_db);
}


TransactionPtr EquationalDatabase::begin_transaction(const std::string& query_text)
{
	sqlite3_stmt* stmt = nullptr;

	const int rc = sqlite3_prepare_v2(m_db, query_text.c_str(), -1,
		&stmt, nullptr);

	if (rc != SQLITE_OK || stmt == nullptr)
	{
		ATP_DATABASE_ASSERT(stmt == nullptr);

		return nullptr;
	}

	return std::make_unique<SQLiteQueryTransaction>(m_db, stmt);
}


boost::optional<std::string>
EquationalDatabase::model_context_filename(
	const std::string& model_context_name)
{
	sqlite3_stmt* p_stmt = nullptr;
	int rc;

	std::stringstream query_builder;
	query_builder << "SELECT filename FROM model_contexts WHERE"
		<< " name = " << '"' << model_context_name << '"' << ';';
	const std::string query_str = query_builder.str();

	rc = sqlite3_prepare_v2(m_db, query_str.c_str(), -1, &p_stmt,
		nullptr);

	if (rc != SQLITE_OK || p_stmt == nullptr)
	{
		ATP_DATABASE_ASSERT(p_stmt == nullptr);

		return boost::none;
	}

	rc = sqlite3_step(p_stmt);
	boost::optional<std::string> result = boost::none;

	if (rc == SQLITE_ROW)
	{
		auto type = sqlite3_column_type(p_stmt, 0);

		if (type == SQLITE_TEXT)
		{
			result = std::string((const char*)
				sqlite3_column_text(p_stmt, 0));
		}
	}
	// else no results
	// warning: if rc == SQLITE_BUSY then we failed to obtain a lock!

	sqlite3_finalize(p_stmt);

	return result;
}


boost::optional<size_t> EquationalDatabase::model_context_id(
	const std::string& model_context_name)
{
	sqlite3_stmt* p_stmt = nullptr;
	int rc;

	std::stringstream query_builder;
	query_builder << "SELECT ctx_id FROM model_contexts WHERE"
		<< " name = " << '"' << model_context_name << '"' << ';';
	const std::string query_str = query_builder.str();

	rc = sqlite3_prepare_v2(m_db, query_str.c_str(), -1, &p_stmt,
		nullptr);

	if (rc != SQLITE_OK || p_stmt == nullptr)
	{
		ATP_DATABASE_ASSERT(p_stmt == nullptr);

		return boost::none;
	}

	rc = sqlite3_step(p_stmt);
	boost::optional<size_t> result = boost::none;

	if (rc == SQLITE_ROW)
	{
		const auto type = sqlite3_column_type(p_stmt, 0);

		if (type == SQLITE_INTEGER)
		{
			const int maybe_result = sqlite3_column_int(p_stmt, 0);
			
			if (maybe_result < 0)
				return boost::none;  // bad

			result = (size_t)maybe_result;
		}
	}
	// else no results
	// warning: if rc == SQLITE_BUSY then we failed to obtain a lock!

	sqlite3_finalize(p_stmt);

	return result;
}


TransactionPtr EquationalDatabase::get_theorems_for_kernel_transaction(
	size_t ctx_id,
	const logic::ModelContextPtr& p_ctx,
	const logic::StatementArrayPtr& targets)
{
	// limit on number returned (arbitrary, for now)
	static const size_t N = 25;

	std::stringstream query_builder;

	query_builder << "SELECT stmt FROM theorems"
		" WHERE id >= "
		"(abs(random()) % (SELECT max(id) FROM theorems)) AND"
		"ctx_id = " << ctx_id << "LIMIT " << N << ";";

	return begin_transaction(query_builder.str());
}


TransactionPtr EquationalDatabase::finished_proof_attempt_transaction(
	size_t ctx_id,
	const logic::ModelContextPtr& p_ctx,
	const std::vector<atp::logic::ProofStatePtr>& proof_states,
	const std::vector<float>& proof_times,
	const std::vector<size_t>& max_mem_usages,
	const std::vector<size_t>& num_node_expansions)
{
	std::stringstream query_builder;
	query_builder << "BEGIN TRANSACTION;";

	for (size_t i = 0; i < proof_states.size(); i++)
	{
		// add the theorem to the database if it's not in there
		// already
		query_builder << "INSERT OR REPLACE INTO theorems (stmt, "
			<< "ctx) VALUES (" << proof_states[i]->target_stmt().to_str()
			<< ", " << ctx_id << "); ";

		// a query for finding the theorem ID of the target statement
		const std::string find_thm_id =
			"(SELECT thm_id FROM theorems WHERE stmt = \"" +
			proof_states[i]->target_stmt().to_str() + "\")";

		// add proof attempt to database
		query_builder << "INSERT INTO proof_attempts (thm_id, "
			<< "time_cost, max_mem, num_expansions) VALUES ("
			<< find_thm_id << ", "
			<< proof_times[i] << ", " << max_mem_usages[i]
			<< ", " << num_node_expansions[i] << "); ";

		// IF THE PROOF WAS SUCCESSFUL, add the proof to the database
		if (proof_states[i]->completion_state() ==
			atp::logic::ProofCompletionState::PROVEN)
		{
			query_builder << "INSERT INTO proofs (thm_id, proof) "
				<< "VALUES (" << find_thm_id << ", \""
				<< proof_states[i]->to_str() << "\"); ";
		}
		// obviously we do not want to do this if it failed ^
	}

	query_builder << "COMMIT;";

	return begin_transaction(query_builder.str());
}


}  // namespace db
}  // namespace atp


