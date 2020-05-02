/**
\file

\author Samuel Barrett

*/


#include "EquationalDatabase.h"
#include <sstream>
#include <sqlite3.h>
#include <boost/filesystem.hpp>


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


boost::optional<std::string>
EquationalDatabase::model_context_filename(
	const std::string& model_context_name)
{
	sqlite3_stmt* p_stmt = nullptr;
	int rc;

	std::stringstream query_builder;
	query_builder << "SELECT filename FROM model_contexts WHERE"
		<< " name = " << '"' << model_context_name << '"' << ';';
	std::string query_str = query_builder.str();

	rc = sqlite3_prepare_v2(m_db, query_str.c_str(), -1, &p_stmt,
		nullptr);

	if (rc != SQLITE_OK)
	{
		ATP_DATABASE_ASSERT(p_stmt == nullptr);

		return boost::none;
	}

	if (p_stmt == nullptr)
	{
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


}  // namespace db
}  // namespace atp


