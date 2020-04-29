#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an index which just stores the rows in a contiguous
	array.

*/


#include <boost/optional.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/DBContainers.h"
#include "../Interfaces/IBufferManager.h"


namespace atp
{
namespace db
{


/**
\brief An index which stores the table row by row (however actually
	stores it column-major).

\tparam StmtT The concrete statement type of the logic to be used.

\details This index tries to store the data as an array of column
	arrays, rather than an array of rows, however since we are
	limited to a single file it can make it tricky for inserting
	new elements. Hence we let the file contain several *blocks*,
	where each block contains a column array for each column, and
	each block has a capacity. Once the capacity has been reached,
	we add a new block to the end of the file.
*/
template<typename StmtT>
class BasicArrayIndex :
	public IDBColumnDecoratorContainer
{
private:
	struct Block
	{
		std::vector<size_t> cur_col_sizes,
			col_capacities;
	};

public:
	BasicArrayIndex(IBufferManager& buf_mgr, ResourceName my_name,
		const std::vector<std::string>& col_names,
		const std::vector<DType>& col_types) :
		m_buf_mgr(buf_mgr), m_name(my_name), m_cols(my_cols)
	{ }

	inline ColumnList cols() const override
	{
		return ColumnList(&m_col_names);
	}
	inline size_t num_cols() const override
	{
		return m_cols.size();
	}
	QuerySupport get_support(
		const DBContainerQuery& q) const override;
	std::shared_ptr<IDBIterator> begin_query(
		const DBContainerQuery& q) override;
	ResourceList get_resources(
		const DBContainerQuery& q) const override
	{
		ResourceList res_list;
		res_list.push_back(m_name);
		return res_list;
	}
	bool has_autokey_col() const override;
	Column get_autokey_col() const override;
	bool has_col_flag(ColumnFlag cf,
		const Column& col) const override;

private:
	IBufferManager& m_buf_mgr;
	const ResourceName m_name;

	std::vector<std::string> m_col_names;
	std::vector<DType> m_col_types;
	boost::optional<size_t>
		m_autokey_col;  // index of the autokey col, if it exists
	std::vector<bool>
		m_col_unique;  // whether each col is unique

};


}  // namespace db
}  // namespace atp


