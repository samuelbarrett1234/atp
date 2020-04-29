/**
\file

\author Samuel Barrett

*/


#include <numeric>
#include "BasicArrayIndexIterators.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


size_t fit_rows(size_t excess_capacity,
	const std::vector<size_t>& off_array,
	size_t next_row_to_insert)
{
	// the amount of memory inserted so far
	const size_t mem_used = (next_row_to_insert > 0) ?
		off_array[next_row_to_insert - 1] : 0;

	auto iter = std::lower_bound(off_array.begin(),
		off_array.end(), mem_used + excess_capacity);

	// CHECK!
	// This is almost certainly not correct :)

	ATP_DATABASE_ASSERT(iter == off_array.end() ||
		*iter - mem_used <= excess_capacity);

	return std::distance(off_array.begin(), iter);
}


IteratorBase::IteratorBase(
	std::shared_ptr<ILock> p_lock,
	std::shared_ptr<IReadableStream> p_stream) :
	m_lock(std::move(p_lock)),
	m_stream(std::move(p_stream)),
	m_rw_stream(dynamic_cast<IReadWriteStream*>(m_stream.get())),
	m_cur_idx_in_block(0)
{
	m_cur_block.read(is());
}


InsertIterator::InsertIterator(
	std::shared_ptr<ILock> p_lock,
	std::shared_ptr<IReadableStream> p_stream,
	const std::vector<DArray>& rows) :
	IteratorBase(std::move(p_lock), std::move(p_stream)),
	m_num_cols(rows.size()), m_input_row_idx(0),
	m_num_rows_to_insert(rows.empty() ? 0 : rows.front().size()),
	m_end_block_offset(0)  // will properly init this: `bring_to_end`
{
	serialise_insert_data(rows);
	bring_to_end();
}


void InsertIterator::advance()
{
	ATP_DATABASE_PRECOND(valid());

	// skip the current row
	++m_input_row_idx;
}


bool InsertIterator::valid() const
{
	return (m_input_row_idx == m_num_rows_to_insert);
}


void InsertIterator::insert_advance()
{
	// compute, over all columns, the smallest number of rows that
	// would fit into the current block
	size_t min_num_rows_fit = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < m_num_cols; ++i)
	{
		// num bytes of space left for this col
		const size_t excess_capacity =
			m_cur_block.col_total_capacities[i] -
			m_cur_block.cur_col_sizes[i];

		// the amount of memory we have read in so far
		const size_t mem_read_so_far =
			m_col_element_offs[i][m_input_row_idx];

		ATP_DATABASE_ASSERT(m_cur_block.cur_col_sizes[i] ==
			mem_read_so_far);

		const size_t num_rows_fit = fit_rows(excess_capacity,
			m_col_element_offs[i], m_input_row_idx);

		min_num_rows_fit = std::min(min_num_rows_fit,
			num_rows_fit);
	}

	// if min_num_rows_fit == 0 then create a new block



	// now insert the required number of rows


}


void InsertIterator::bring_to_end()
{
}


void InsertIterator::serialise_insert_data(
	const std::vector<DArray>& rows)
{
	// don't forget to fill in the autokey column!
}


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


