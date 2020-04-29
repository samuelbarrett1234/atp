#pragma once


/**
\file

\author Samuel Barrett

\brief A container representing a table for equational logic
	statements.

*/


#include <map>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ATPLogic.h>
#include "../../ATPDatabaseAPI.h"
#include "../../Interfaces/DBContainers.h"


namespace atp
{
namespace db
{
namespace equational
{


/**
\brief Represents an equational logic table

\warning This class needs to be thread safe!
*/
class ATP_DATABASE_API Table :
	public IDBColumnDecoratorContainer
{
public:
	/**
	\brief Create a table object from a config file.

	\param p_lang The equational language object shared across the DB

	\param cfg_in The root of the table node in the config file

	\param target_dir The folder where we should look for the table's
		data files, if any.

	\param out_res_files Any resource file that this table may need
		will get added to this mapping, so it can be placed in a file
		buffer manager. The assigned resource names will be
		arbitrary.
	*/
	static std::unique_ptr<Table> load_from_config(
		const logic::LanguagePtr& p_lang,
		const boost::property_tree::ptree& cfg_in,
		const std::string& target_dir,
		std::map<ResourceName, std::string>& out_res_files
	);

public:  // interface functions

	inline ColumnList cols() const override
	{
		return ColumnList(&m_col_names);
	}
	inline size_t num_cols() const override
	{
		return m_col_names.size();
	}
	QuerySupport get_support(
		const DBContainerQuery& q) const override;
	std::shared_ptr<IDBIterator> begin_query(
		const DBContainerQuery& q) override;
	inline ResourceList get_resources(
		const DBContainerQuery& q) const override
	{
		return m_res_list;
	}
	inline bool has_autokey_col() const override
	{
		return m_autokey_col.has_value();
	}
	inline Column get_autokey_col() const override
	{
		return Column(*m_autokey_col, &m_col_names);
	}
	bool has_col_flag(ColumnFlag cf,
		const Column& col) const override;

public:  // other functions

	inline const std::string& name() const
	{
		return m_name;
	}

private:
	std::string m_name;

	std::vector<std::string> m_col_names;
	std::vector<DType> m_col_types;
	boost::optional<size_t>
		m_autokey_col;  // index of the autokey col, if it exists
	std::vector<bool>
		m_col_unique;  // whether each col is unique

	ResourceList m_res_list;  // all resources for this table

	// invariant: all these indices have the same number of rows!
	std::vector<std::unique_ptr<IDBInsertableContainer>> m_indices;

	logic::LanguagePtr m_lang;
	logic::ModelContextPtr m_ctx;
};


}  // namespace equational
}  // namespace db
}  // namespace atp


