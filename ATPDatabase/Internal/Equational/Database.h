#pragma once


/**
\file

\author Samuel Barrett

\brief IDatabase implementation for equational logic

*/


#include <istream>
#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include "../../ATPDatabaseAPI.h"
#include "../../Interfaces/IDatabase.h"
#include "../../Interfaces/IBufferManager.h"
#include "../StrictLockManager.h"
#include "../BasicFileBufferManager.h"


/**
\namespace atp::db::equational

\brief Contains all databases and operations relating to equational
	logic structures.
*/


namespace atp
{
namespace db
{
namespace equational
{


/**
\brief Represents a database which handles with equational logic
	statements only.
*/
class ATP_DATABASE_API Database :
	public IDatabase
{
public:  // builder functions

	/**
	\brief Test whether the given config file is "one of ours"

	\note This does not check that the entire file is valid, only
		that its "type" is an equational config file.
	*/
	static bool is_my_kind_of_config(
		const boost::property_tree::ptree& cfg_in);

	/**
	\brief Build an equational database from the given input schema,
		and writing to the given output configuration file.

	\pre The schema must have "logic"=="EQUATIONAL" in the root of
		the ptree.

	\warning Schema files are uniform across different database types
		however config files are specific to the database type, and
		thus are subject to arbitrary changes in the implementation.
		Thus one should not rely on the config file format to remain
		constant or even uniform across different DB types.
	*/
	static DatabasePtr create_from_schema(
		const boost::property_tree::ptree& sch_in,
		boost::property_tree::ptree& cfg_out);

	/**
	\brief Build an equational database from the given input
		configuration file, which was previously created from a
		call to `create_from_schema`, i.e. must be a valid schema
		for this type.

	\pre is_my_kind_of_config(cfg_in)
	*/
	static DatabasePtr load_from_config(
		const boost::property_tree::ptree& cfg_in);

public:  // interface functions

	inline std::string name() const override
	{
		return m_name;
	}
	inline std::string description() const override
	{
		return m_desc;
	}
	inline ILockManager& lock_mgr() override
	{
		return m_lk_mgr;
	}
	inline logic::LangType logic_lang() const override
	{
		return logic::LangType::EQUATIONAL_LOGIC;
	}

private:
	std::string m_name, m_desc;
	StrictLockManager m_lk_mgr;
	std::unique_ptr<IBufferManager> m_buf_mgr;
};


}  // namespace equational
}  // namespace db
}  // namespace atp


