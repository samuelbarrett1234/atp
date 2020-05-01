/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndexUpdateIterator.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


UpdateIterator::UpdateIterator(const logic::ILanguage& lang,
	const logic::IModelContext& ctx, const BasicArrayIndex& parent,
	std::shared_ptr<ILock> p_lock,
	std::shared_ptr<IReadableStream> p_stream,
	const std::vector<DType>& col_types,
	const std::vector<std::string>& col_names) :
	SelectAllIterator(lang, ctx, parent, std::move(p_lock),
		std::move(p_stream), col_types, col_names)
{ }


void UpdateIterator::set(const Column& col, const DValue& value)
{
	ATP_DATABASE_PRECOND(valid());
	ATP_DATABASE_ASSERT(!m_row.empty());

	const auto col_list = SelectAllIterator::cols();

	ATP_DATABASE_PRECOND(col_list.contains(col));

	const size_t idx = col_list.index_of(col);

	set(idx, value);
}


void UpdateIterator::set_all(const std::vector<DValue>& values)
{
	ATP_DATABASE_PRECOND(valid());
	ATP_DATABASE_ASSERT(!m_row.empty());

	ATP_DATABASE_PRECOND(values.size() == m_row.size());

	// there isn't really a more efficient way of doing this:
	for (size_t i = 0; i < values.size(); ++i)
	{
		set(i, values[i]);
	}
}


void UpdateIterator::set(size_t idx, const DValue& value)
{
	ATP_DATABASE_PRECOND(idx < m_row.size());
	ATP_DATABASE_ASSERT(m_row.size() == m_col_types.size());
	ATP_DATABASE_PRECOND(value.type() == m_col_types[idx]);

	m_row[idx] = value;

	os().seekp(m_cur_block_end_off + m_cur_block.col_elem_offset(idx,
		m_cur_idx_in_block));

	// TODO: write the data, but what if it changes the size???
}


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


