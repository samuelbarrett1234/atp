#pragma once


/**
\file

\author Samuel Barrett

\brief IDatabase implementation for equational logic

*/


#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IDatabase.h"


struct sqlite3;  // forward declaration


namespace atp
{
namespace db
{


/**
\brief Represents a database which handles with equational logic
	statements only.
*/
class ATP_DATABASE_API EquationalDatabase :
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
	~EquationalDatabase();

	inline std::string name() const override
	{
		return m_name;
	}
	inline std::string description() const override
	{
		return m_desc;
	}
	inline logic::LanguagePtr logic_lang() const override
	{
		return m_lang;
	}

	boost::optional<std::string> model_context_filename(
		const std::string& model_context_name) override;

private:
	std::string m_name, m_desc;
	logic::LanguagePtr m_lang;
	sqlite3* m_db;
};


}  // namespace db
}  // namespace atp


