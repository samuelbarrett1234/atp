#pragma once


/**
\file

\author Samuel Barrett

\brief IDatabase implementation for equational logic

*/


#include <sqlite3.h>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IDatabase.h"


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

public:  // interface functions

	inline std::string name() const override
	{
		return m_name;
	}
	inline std::string description() const override
	{
		return m_desc;
	}
	inline logic::LangType logic_lang() const override
	{
		return logic::LangType::EQUATIONAL_LOGIC;
	}

private:
	std::string m_name, m_desc;
	logic::LanguagePtr m_lang;
};


}  // namespace db
}  // namespace atp


