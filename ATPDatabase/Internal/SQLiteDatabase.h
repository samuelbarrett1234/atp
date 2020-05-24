#pragma once


/**
\file

\author Samuel Barrett

\brief IDatabase implementation for SQLite databases

*/


#include <map>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IDatabase.h"


struct sqlite3;  // forward declaration


namespace atp
{
namespace db
{


/**
\brief A database which uses SQLite internally
*/
class ATP_DATABASE_API SQLiteDatabase :
	public IDatabase
{
public:  // builder functions
	
	/**
	\brief Create a new database object from the given SQLite database
		filename.

	\returns A valid database object if the SQL database construction was
		successful, or nullptr if the filename was incorrect / the file
		was invalid.
	*/
	static DatabasePtr load_from_file(
		const std::string& filename);

public:
	~SQLiteDatabase();

	inline std::string name() const override
	{
		return m_name;
	}
	inline std::string description() const override
	{
		return m_desc;
	}

	TransactionPtr begin_transaction(
		const std::string& query_text) override;

	QueryBuilderPtr create_query_builder(
		QueryBuilderType qb_type) override;

	boost::optional<std::string> model_context_filename(
		const std::string& model_context_name) override;

	boost::optional<size_t> model_context_id(
		const std::string& model_context_name) override;

	boost::optional<std::string> search_settings_filename(
		const std::string& search_settings_name) override;

	boost::optional<size_t> search_settings_id(
		const std::string& search_settings_name) override;

private:
	std::map<std::string, std::string> m_query_templates;
	std::string m_name, m_desc;
	sqlite3* m_db;
};


}  // namespace db
}  // namespace atp


