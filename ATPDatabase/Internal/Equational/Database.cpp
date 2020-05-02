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
		p_db->m_target_dir = cfg_in.get<std::string>("target",
				"");

		if (!boost::filesystem::is_directory(p_db->m_target_dir))
		{
			return nullptr;
		}

		// setup logic language
		p_db->m_lang = logic::create_language(
			logic::LangType::EQUATIONAL_LOGIC);

		// we will fill this in as we go, and create a file buffer
		// manager out of it at the end
		std::map<ResourceName, std::string> res_files;

		// every DB needs tables, so this throws if there isn't a
		// "tables" value, but would not throw if "tables" was empty.
		const auto tables = cfg_in.get_child("tables");

		// load each table
		for (const auto table_data : tables)
		{
			p_db->m_tables.emplace_back(Table::load_from_config(
				p_db->m_lang, table_data.second, p_db->m_target_dir,
				res_files));

			if (p_db->m_tables.back() == nullptr)
				return nullptr;  // this table failed to load

			// if the table we just created uses a name that we have
			// already used,
			if (std::find_if(std::next(p_db->m_tables.rbegin()),
				p_db->m_tables.rend(), [&p_db](const auto& p_tab) -> bool
				{ return p_tab->name() == p_db->m_tables.back()->name(); })
				!= p_db->m_tables.rend())
				return nullptr;
		}

		if (p_db->m_tables.empty())
			return nullptr;  // cannot have DB with no tables

		// create file buffer manager
		p_db->m_buf_mgr = std::make_unique<BasicFileBufferManager>(
			std::move(res_files));

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


