/**
\file

\author Samuel Barrett

*/


#include "SQLiteDatabase.h"
#include <sstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include "SQLiteTransaction.h"
#include "TransactionListWrapper.h"
#include "SQLiteRndThmSelectQryBder.h"
#include "SQLiteSaveProofResultsQryBder.h"
#include "SQLiteCheckAxInDbQryBder.h"
#include "SQLiteFindHmmConjModel.h"
#include "SQLiteGetHmmConjStTransParams.h"
#include "SQLiteGetHmmConjObsParams.h"


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
	// we better check this!
	if (sqlite3_threadsafe() == 0)
	{
		ATP_DATABASE_LOG(fatal) << "SQLite3 is not thread safe! "
			"Please find a thread-safe version of the library. "
			"Aborting...";
		return nullptr;
	}

	if (!boost::filesystem::is_regular_file(filename))
		return nullptr;

	auto p_db = std::make_shared<SQLiteDatabase>();
	int rc;

	rc = sqlite3_open(filename.c_str(), &p_db->m_db);
	if (rc != 0)
	{
		ATP_DATABASE_LOG(error)
			<< "Failed to open SQLite database \"" << filename <<
			"\", error code " << rc;
			
		return nullptr;
	}
	ATP_DATABASE_ASSERT(p_db->m_db != nullptr);

	return p_db;
}


SQLiteDatabase::~SQLiteDatabase()
{
	ATP_DATABASE_LOG(debug) << "Closing SQLite database...";
	sqlite3_close(m_db);
}


TransactionPtr SQLiteDatabase::begin_transaction(
	const std::string& query_text)
{
	ATP_DATABASE_LOG(trace) << "Beginning a transaction...";

	// warning: there may be many transactions involved here!

	const char* p_begin = query_text.data();
	const char* p_cur = p_begin;
	const char* p_end = p_begin + query_text.size();

	// we are done iff there is only whitespace between p_cur and
	// p_end
	auto is_done = [&p_cur, &p_end]() -> bool
	{
		return std::find_if(p_cur, p_end,
			[](char c) { return std::isspace(c) == 0; })
			== p_end;
	};

	auto trans_list_builder = TransactionListWrapper::Builder();

	while (!is_done())
	{
		sqlite3_stmt* stmt = nullptr;

		const char* p_last_cur = p_cur;
		const int rc = sqlite3_prepare_v2(m_db, p_cur, -1,
			&stmt, &p_cur);

		if (rc != SQLITE_OK || stmt == nullptr)
		{
			ATP_DATABASE_LOG(error)
				<< "Failed to build SQLite statement from: \""
				<< std::string(p_cur, p_end)
				<< "\".";

			ATP_DATABASE_ASSERT(stmt == nullptr);

			ATP_DATABASE_LOG(error) << "Error code " << rc <<
				" returned from SQLite query; error message: "
				<< sqlite3_errmsg(m_db);

			return nullptr;
		}
		else ATP_DATABASE_LOG(debug) << "Successfully built SQLite "
			"statement from \"" << std::string(p_last_cur, p_cur)
			<< "\".";

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


QueryBuilderPtr SQLiteDatabase::create_query_builder(
	QueryBuilderType qb_type)
{
	switch (qb_type)
	{
	case QueryBuilderType::RANDOM_THM_SELECTION:
		return std::make_unique<SQLiteRndThmSelectQryBder>();
	case QueryBuilderType::SAVE_THMS_AND_PROOFS:
		return std::make_unique<SQLiteSaveProofResultsQryBder>();
	case QueryBuilderType::CHECK_AXIOMS_IN_DB:
		return std::make_unique<SQLiteCheckAxInDbQryBder>();
	case QueryBuilderType::FIND_HMM_CONJ_MODEL:
		return std::make_unique<SQLiteFindHmmConjModel>();
	case QueryBuilderType::GET_HMM_CONJ_ST_TRANS_PARAMS:
		return std::make_unique<SQLiteGetHmmConjStTransParams>();
	case QueryBuilderType::GET_HMM_CONJ_OBS_PARAMS:
		return std::make_unique<SQLiteGetHmmConjObsParams>();
	default:
		ATP_DATABASE_PRECOND(false && "bad type!");
		return nullptr;
	}
}


boost::optional<std::string>
SQLiteDatabase::model_context_filename(
	const std::string& model_context_name)
{
	ATP_DATABASE_LOG(debug) << "Querying SQLite database for model"
		" context name \"" << model_context_name << "\".";

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
	ATP_DATABASE_LOG(debug) << "Querying SQLite database for model"
		" context name \"" << model_context_name << "\".";

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
	ATP_DATABASE_LOG(debug) << "Querying SQLite database for model"
		" context name \"" << search_settings_name << "\".";

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
	ATP_DATABASE_LOG(debug) << "Querying SQLite database for model"
		" context name \"" << search_settings_name << "\".";

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
		switch (p_query->state())
		{
		case TransactionState::RUNNING:
			ATP_DATABASE_LOG(warning) << "SQLite query \"" << query
				<< "\" did not finish after being stepped "
				<< N << " times.";
			break;
		case TransactionState::COMPLETED:
			ATP_DATABASE_LOG(warning) << "SQLite query \"" << query
				<< "\" turned up no results, even though it "
				"finished.";
			break;
		case TransactionState::FAILED:
			ATP_DATABASE_LOG(warning) << "SQLite query \"" << query
				<< "\" failed.";
			break;
		}

		return boost::none;
	}
}


}  // namespace db
}  // namespace atp


