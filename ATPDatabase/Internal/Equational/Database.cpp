/**
\file

\author Samuel Barrett

*/


#include "Database.h"
#include <boost/filesystem.hpp>


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

	try
	{
		p_db->m_name = cfg_in.get<std::string>("name",
			"Unnamed Database");
		p_db->m_desc = cfg_in.get<std::string>("desc",
			"[No description]");

		// setup logic language
		p_db->m_lang = logic::create_language(
			logic::LangType::EQUATIONAL_LOGIC);



		// now we are done!
	}
	catch (boost::property_tree::ptree_error&)
	{
		return nullptr;
	}

	return p_db;
}


}  // namespace equational
}  // namespace db
}  // namespace atp


