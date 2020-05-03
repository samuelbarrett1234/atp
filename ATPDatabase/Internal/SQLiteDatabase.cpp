/**
\file

\author Samuel Barrett

*/


#include "SQLiteDatabase.h"
#include <sstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include "SQLiteTransaction.h"
#include "TransactionListWrapper.h"


namespace atp
{
namespace db
{


/**
\brief Helper function for queries which just return a single value
*/
boost::optional<DValue> try_get_value(SQLiteDatabase* db,
	const std::string& query, DType dtype);


DatabasePtr SQLiteDatabase::load_from_file(
	const std::string& filename)
{
	if (!boost::filesystem::is_regular_file(filename))
		return nullptr;

	auto p_db = std::make_shared<SQLiteDatabase>();
	int rc;

	rc = sqlite3_open(filename.c_str(), &p_db->m_db);
	if (rc != 0)
	{
		return nullptr;
	}
	ATP_DATABASE_ASSERT(p_db->m_db != nullptr);

	return p_db;
}


SQLiteDatabase::~SQLiteDatabase()
{
	sqlite3_close(m_db);
}


TransactionPtr SQLiteDatabase::begin_transaction(
	const std::string& query_text)
{
	// warning: there may be many transactions involved here!

	const char* p_begin = query_text.data();
	const char* p_cur = p_begin;
	const char* p_end = p_begin + query_text.size();

	auto trans_list_builder = TransactionListWrapper::Builder();

	while (p_cur != p_end)
	{
		sqlite3_stmt* stmt = nullptr;

		const int rc = sqlite3_prepare_v2(m_db, p_cur, -1,
			&stmt, &p_cur);

		if (rc != SQLITE_OK || stmt == nullptr)
		{
			ATP_DATABASE_ASSERT(stmt == nullptr);

			return nullptr;
		}

		auto p_trans = std::make_unique<
			SQLiteQueryTransaction>(m_db, stmt);

		if (p_trans == nullptr)
			return nullptr;

		trans_list_builder.add(std::move(p_trans));
	}

	if (trans_list_builder.m_transaction_list.size() == 1)
	{
		return std::move(
			trans_list_builder.m_transaction_list.front());
	}
	else
	{
		return trans_list_builder.build();
	}
}


boost::optional<std::string>
SQLiteDatabase::model_context_filename(
	const std::string& model_context_name)
{
	std::stringstream query_builder;
	query_builder << "SELECT filename FROM model_contexts WHERE"
		<< " name = " << '"' << model_context_name << '"' << ';';

	auto value = try_get_value(this, query_builder.str(),
		DType::STR);

	if (!value.has_value())
		return boost::none;
	else
		return get_str(*value);
}


boost::optional<size_t> SQLiteDatabase::model_context_id(
	const std::string& model_context_name)
{
	std::stringstream query_builder;
	query_builder << "SELECT ctx_id FROM model_contexts WHERE"
		<< " name = " << '"' << model_context_name << '"' << ';';

	auto value = try_get_value(this, query_builder.str(),
		DType::INT);

	if (!value.has_value() || get_int(*value) < 0)
		return boost::none;
	else
		return (size_t)get_int(*value);
}


boost::optional<std::string> SQLiteDatabase::search_settings_filename(
	const std::string& search_settings_name)
{
	std::stringstream query_builder;
	query_builder << "SELECT filename FROM search_settings WHERE"
		<< " name = " << '"' << search_settings_name << '"' << ';';

	auto value = try_get_value(this, query_builder.str(),
		DType::STR);

	if (!value.has_value())
		return boost::none;
	else
		return get_str(*value);
}


boost::optional<size_t> SQLiteDatabase::search_settings_id(
	const std::string& search_settings_name)
{
	std::stringstream query_builder;
	query_builder << "SELECT ss_id FROM search_settings WHERE"
		<< " name = " << '"' << search_settings_name << '"' << ';';

	auto value = try_get_value(this, query_builder.str(),
		DType::INT);

	if (!value.has_value() || get_int(*value) < 0)
		return boost::none;
	else
		return (size_t)get_int(*value);
}


TransactionPtr SQLiteDatabase::get_theorems_for_kernel_transaction(
	size_t ctx_id, size_t ss_id,
	const logic::ModelContextPtr& p_ctx,
	const logic::StatementArrayPtr& targets)
{
	// limit on number returned (arbitrary, for now)
	static const size_t N = 25;

	std::stringstream query_builder;

	// IMPORTANT: only load theorems for which there exist proofs!
	query_builder << "SELECT stmt FROM theorems JOIN proofs ON thm_id=id"
		" WHERE ctx = " << ctx_id <<
		" ORDER BY RANDOM() LIMIT " << N << ";";

	return begin_transaction(query_builder.str());
}


TransactionPtr SQLiteDatabase::finished_proof_attempt_transaction(
	size_t ctx_id, size_t ss_id,
	const logic::ModelContextPtr& p_ctx,
	const logic::StatementArrayPtr& targets,
	const std::vector<atp::logic::ProofStatePtr>& proof_states,
	const std::vector<float>& proof_times,
	const std::vector<size_t>& max_mem_usages,
	const std::vector<size_t>& num_node_expansions)
{
	ATP_DATABASE_PRECOND(targets->size() ==
		proof_states.size());
	ATP_DATABASE_PRECOND(proof_times.size() ==
		proof_states.size());
	ATP_DATABASE_PRECOND(max_mem_usages.size() ==
		proof_states.size());
	ATP_DATABASE_PRECOND(num_node_expansions.size() ==
		proof_states.size());

	std::stringstream query_builder;
	query_builder << "BEGIN TRANSACTION;\n\n";

	for (size_t i = 0; i < proof_states.size(); i++)
	{
		// add the theorem to the database if it's not in there
		// already
		query_builder << "INSERT INTO theorems (stmt, "
			<< "ctx) SELECT '" << targets->at(i).to_str()
			<< "', " << ctx_id << " WHERE NOT EXISTS("
			<< "SELECT 1 FROM theorems WHERE stmt == '"
			<< targets->at(i).to_str() << "');\n\n";

		// a query for finding the theorem ID of the target statement
		const std::string find_thm_id =
			"(SELECT id FROM theorems WHERE stmt = '" +
			targets->at(i).to_str() + "')";

		// add proof attempt to database
		query_builder << "INSERT INTO proof_attempts(thm_id, ss_id, "
			<< "time_cost, max_mem, num_expansions) VALUES ("
			<< find_thm_id << ", " << ss_id << ", "
			<< proof_times[i] << ", " << max_mem_usages[i]
			<< ", " << num_node_expansions[i] << ");\n\n";

		// IF THE PROOF WAS SUCCESSFUL, add the proof to the database
		if (proof_states[i] != nullptr &&
			proof_states[i]->completion_state() ==
			atp::logic::ProofCompletionState::PROVEN)
		{
			// make sure to not try inserting a new proof if there
			// is already a proof in the database!
			query_builder << "INSERT INTO proofs (thm_id, proof) "
				<< "SELECT " << find_thm_id << " , '"
				<< proof_states[i]->to_str() << "' WHERE NOT EXISTS"
				<< " (SELECT 1 FROM proofs JOIN theorems ON "
				<< "thm_id=id WHERE stmt = '"
				<< targets->at(i).to_str() << "');\n\n";
		}
		// obviously we do not want to do this if it failed ^
	}

	query_builder << "COMMIT;";

	return begin_transaction(query_builder.str());
}


boost::optional<DValue> try_get_value(SQLiteDatabase* db,
	const std::string& query, DType dtype)
{
	auto p_trans = db->begin_transaction(query);

	if (p_trans == nullptr)
	{
		return boost::none;  // query bad? DB must be in a bad state
	}

	auto p_query = dynamic_cast<IQueryTransaction*>(p_trans.get());
	ATP_DATABASE_ASSERT(p_query != nullptr);

	// get query into a ready position (try at most N times)
	static const size_t N = 100;
	for (size_t i = 0; i < N && p_query->state() ==
		TransactionState::RUNNING && !p_query->has_values(); ++i)
		p_query->step();

	if (p_query->state() ==
		TransactionState::RUNNING && p_query->has_values()
		&& p_query->arity() > 0)
	{
		DValue dv;
		if (p_query->try_get(0, dtype, &dv))
		{
			return dv;
		}
		else
		{
			return boost::none;
		}
	}
	else
	{
		return boost::none;
	}
}


}  // namespace db
}  // namespace atp


