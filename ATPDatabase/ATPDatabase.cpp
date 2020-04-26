/**
\file

\author Samuel Barrett

*/


#include "ATPDatabase.h"
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Internal/Equational/Database.h"


using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;


namespace atp
{
namespace db
{


ATP_DATABASE_API DatabasePtr create_from_schema(
	std::istream& in_json_schema,
	std::ostream& out_db_config_file)
{
	ptree sch, cfg;
	DatabasePtr db;

	read_json(in_json_schema, sch);

	const std::string db_type = sch.get<std::string>("logic", "");

	if (db_type == "EQUATIONAL")
	{
		db = equational::Database::create_from_schema(sch, cfg);
	}
	// else if ... [other DB types]

	if (db != nullptr)
	{
		// save the config if we successfully got a database

		write_json(out_db_config_file, cfg);
	}

	return db;
}


ATP_DATABASE_API DatabasePtr load_from_config_file(
	std::istream& in_db_config_file)
{
	ptree cfg;

	read_json(in_db_config_file, cfg);

	// try to figure out whose config type `cfg` is:
	if (equational::Database::is_my_kind_of_config(cfg))
	{
		return equational::Database::load_from_config(cfg);
	}

	// oops! this was nobody's kind of config
	return nullptr;
}


logic::StatementArrayPtr db_arr_to_stmt_arr(
	const DArray& d_arr)
{
	return DArray::to_stmt_arr(d_arr);
}


}  // namespace db
}  // namespace atp


