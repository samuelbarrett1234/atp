/**
\file

\author Samuel Barrett

*/


#include "Database.h"
#include "../BasicFileBufferManager.h"


namespace atp
{
namespace db
{
namespace equational
{


bool Database::is_my_kind_of_config(
	const boost::property_tree::ptree& cfg_in)
{
	const auto type = cfg_in.get_optional<std::string>("logic");
	return type.has_value() && (*type == "EQUATIONAL");
}


DatabasePtr Database::create_from_schema(
	const boost::property_tree::ptree& sch_in,
	boost::property_tree::ptree& cfg_out)
{
	// note: equational databases have no reason to have a separate
	// config file format, so we keep them the same (but of course
	// this implementation reserves the right to change that)

	cfg_out = sch_in;  // copy
	return load_from_config(cfg_out);
}


DatabasePtr Database::load_from_config(
	const boost::property_tree::ptree& cfg_in)
{
	ATP_DATABASE_PRECOND(is_my_kind_of_config(cfg_in));

	auto p_db = std::make_shared<Database>();



	return p_db;
}


}  // namespace equational
}  // namespace db
}  // namespace atp


