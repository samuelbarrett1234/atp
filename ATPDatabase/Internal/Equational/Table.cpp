/**
\file

\author Samuel Barrett

*/


#include "Table.h"
#include <fstream>
#include <boost/filesystem.hpp>


namespace atp
{
namespace db
{
namespace equational
{


std::unique_ptr<Table> Table::load_from_config(
	const logic::LanguagePtr& p_lang,
	const boost::property_tree::ptree& cfg_in,
	const std::string& target_dir,
	std::map<ResourceName, std::string>& out_res_files)
{
	auto p_table = std::make_unique<Table>();

	try
	{
		p_table->m_name = cfg_in.get<std::string>("name");

		if (p_table->m_name.empty())
			return nullptr;  // tables must have a name

		const std::string model_file = cfg_in.get<std::string>(
			"context");

		if (!boost::filesystem::is_regular_file(model_file))
			return nullptr;  // bad file

		std::ifstream model_in(model_file);

		if (!model_in)
			return nullptr;  // no access to file

		p_table->m_ctx = p_lang->try_create_context(model_in);
		model_in.close();

		if (p_table->m_ctx == nullptr)
			return nullptr;  // syntax error in CTX file?

		// throws an exception if this child doesn't exist
		auto columns = cfg_in.get_child("columns");
		for (auto col : columns)
		{
			const std::string col_name = col.second.get<std::string>(
				"name");

			if (col_name.empty())
				return nullptr;  // bad column name

			const std::string type_name =
				col.second.get<std::string>("type");
			DType type_actual;

			if (type_name == "int")
				type_actual = DType::INT;
			else if (type_name == "uint")
				type_actual = DType::UINT;
			else if (type_name == "float")
				type_actual = DType::FLOAT;
			else if (type_name == "str")
				type_actual = DType::STR;
			else if (type_name == "statement")
				type_actual = DType::STMT;
			else
				return nullptr;  // bad type

			const bool unique = col.second.get<bool>("unique",
				false);
			const bool autokey = col.second.get<bool>("auto-key",
				false);

			if (autokey && p_table->m_autokey_col.has_value())
				return nullptr;  // can't have two autokeys

			p_table->m_col_names.push_back(col_name);
			p_table->m_col_types.push_back(type_actual);
			p_table->m_col_unique.push_back(unique);

			if (autokey)
				// set this to be the index of the back
				p_table->m_autokey_col =
				p_table->m_col_names.size() - 1;
		}

		if (auto indices = cfg_in.get_child_optional("indices"))
		{
			// load indices (throw error if list is empty)

			ATP_DATABASE_ASSERT(false && "TODO!");
			return nullptr;
		}
		else
		{
			// create default index

			ATP_DATABASE_ASSERT(false && "TODO!");
			return nullptr;
		}
	}
	catch (boost::property_tree::ptree_error&)
	{
		return nullptr;
	}

	return p_table;
}


bool Table::has_col_flag(ColumnFlag cf, const Column& col) const
{
	ATP_DATABASE_PRECOND(cols().contains(col));

	const size_t idx = cols().index_of(col);

	switch (cf)
	{
	case ColumnFlag::UNIQUE:
		ATP_DATABASE_ASSERT(idx < m_col_unique.size());
		return m_col_unique[idx];

	case ColumnFlag::AUTO_KEY:
		return (m_autokey_col.has_value() &&
			*m_autokey_col == idx);

	default:
		return false;
	}
}


}  // namespace equational
}  // namespace db
}  // namespace atp


