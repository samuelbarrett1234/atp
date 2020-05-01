/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndexInsertIterator.h"
#include <numeric>
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


InsertIterator::InsertIterator(
	std::shared_ptr<ILock> p_lock,
	std::shared_ptr<IReadableStream> p_stream,
	const std::vector<DArray>& rows,
	const std::vector<DType>& col_types,
	const std::vector<std::string>& col_names) :
	IteratorBase(std::move(p_lock), std::move(p_stream), col_types,
		col_names),
	m_num_cols(rows.size()), m_input_row_idx(0),
	m_num_rows_to_insert(rows.empty() ? 0 : rows.front().size())
{
	serialise_insert_data(rows);
	bring_to_end();
}


void InsertIterator::advance()
{
	ATP_DATABASE_PRECOND(valid());

	// skip each stream ahead by one:
	for (size_t i = 0; i < m_num_cols; ++i)
	{
		const size_t skip_amount = (m_input_row_idx == 0) ?
			m_col_element_offs[i][0] :
			(m_col_element_offs[i][m_input_row_idx] -
				m_col_element_offs[i][m_input_row_idx - 1]);

		ATP_DATABASE_ASSERT(skip_amount > 0);

		m_col_mem_streams[i]->seekp(skip_amount, std::ios::cur);
		m_col_mem_streams[i]->seekg(skip_amount, std::ios::cur);
	}

	// now indicate we've skipped the current row
	++m_input_row_idx;
}


bool InsertIterator::valid() const
{
	return (m_input_row_idx == m_num_rows_to_insert);
}


void InsertIterator::insert_advance()
{
	ATP_DATABASE_PRECOND(valid());

	size_t n = num_rows_that_can_fit();

	// if num rows fit == 0 then create a new block
	if (n == 0)
	{
		// seek to the end, a place to put the new block
		os().seekp(m_cur_block.next_block_offset(),
			std::ios::cur);

#ifdef ATP_DATABASE_DEFENSIVE
		// assert that this would have the same effect as doing
		// os().seekp(0, std::ios::end)
		// this should be the case because m_cur_block is the LAST
		// block!
		const auto pos = os().tellp();
		os().seekp(0, std::ios::end);
		ATP_DATABASE_ASSERT(pos == os().tellp());
#endif

		// the default constructor will create an empty block with
		// capacity for us
		m_cur_block = Block(m_col_types);

		// write empty block data
		m_cur_block.write(os());

		// keep track of where the end of the block is
		const auto block_end_pos = os().tellp();

		// reserve space for us to start inserting
		m_cur_block.reserve_space(os());

		// go back to the end of the block:
		os().seekp(block_end_pos);

		// now recompute this (should no longer be zero!)
		n = num_rows_that_can_fit();
	}

	ATP_DATABASE_ASSERT(n > 0);
	ATP_DATABASE_ASSERT(m_input_row_idx + n <= m_num_rows_to_insert);

	std::vector<char> buffer;

	// now insert the required number of rows

	for (size_t i = 0; i < m_num_cols; ++i)
	{
		// keep track of this position so we can return there later
		const auto block_end_pos = os().tellp();

		// go to place to insert new data

		os().seekp(
			// start of column
			m_cur_block.col_start_offset(i) +
			// skip through existing data
			m_cur_block.column_current_size(i),
			std::ios::cur);

		// how many bytes to transfer between streams?

		const size_t cur_col_pos = (m_input_row_idx > 0) ?
			m_col_element_offs[i][m_input_row_idx - 1] : 0;
		const size_t target_col_pos = m_col_element_offs[i]
			[m_input_row_idx + n - 1];
		const size_t byte_transfer_size =
			target_col_pos - cur_col_pos;

		// perform copy operation:

		buffer.resize(byte_transfer_size);
		m_col_mem_streams[i]->write(buffer.data(), byte_transfer_size);
		os().write(buffer.data(), byte_transfer_size);

		// go back to end of block position:

		os().seekp(block_end_pos, std::ios::beg);
	}

	ATP_DATABASE_ASSERT(false && "need to update the block data (including"
		" number of rows, sizes, etc.)");
}


void InsertIterator::bring_to_end()
{
	// strategy: keep seeking forward until we reach the end of the
	// file, then go back one step

	std::streamoff block_end_off = os().tellp();

	while (os().good())
	{
		// skip over data to arrive at the beginning of the next
		// block
		os().seekp(m_cur_block.total_space());

		if (os().good())
		{
			// bring input stream to the right position
			is().seekg(os().tellp(), std::ios::beg);

			// reload block
			m_cur_block.read(is());

			// update block end position
			block_end_off = os().tellp();

			// the file should still be "good" because blocks always
			// have space reserved after them
			ATP_DATABASE_PRECOND(os().good());
		}
		// else we will quit the while loop
	}

	// restore invariant:
	os().seekp(block_end_off, std::ios::beg);
}


void InsertIterator::serialise_insert_data(
	const std::vector<DArray>& rows)
{
	// don't forget to fill in the autokey column!
	ATP_DATABASE_ASSERT(false && "need to implement autokey column");

	// reserve number of columns
	// (warning: `rows` is stored in column-major order, so rows.size
	// is actually the number of columns)
	m_col_mem_streams.resize(rows.size());
	m_col_element_offs.resize(rows.size());

	for (size_t i = 0; i < rows.size(); ++i)
	{
		// write each column to the streams, storing the sizes of the
		// elements as well
		rows[i].save_raw(*m_col_mem_streams[i],
			&m_col_element_offs[i]);
	}
}


size_t InsertIterator::num_rows_that_can_fit() const
{
	// compute, over all columns, the smallest number of rows that
	// would fit into the current block
	size_t min_num_rows_fit = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < m_num_cols; ++i)
	{
		// num bytes of space left for this col
		const size_t excess_capacity =
			m_cur_block.column_excess_capacity(i);

		// the amount of memory we have read in so far
		const size_t mem_read_so_far =
			m_col_element_offs[i][m_input_row_idx];

		ATP_DATABASE_ASSERT(m_cur_block.column_current_size(i) ==
			mem_read_so_far);

		const size_t num_rows_fit = fit_rows(excess_capacity,
			m_col_element_offs[i], m_input_row_idx);

		min_num_rows_fit = std::min(min_num_rows_fit,
			num_rows_fit);
	}

	return std::min(min_num_rows_fit,
		m_num_rows_to_insert);
}


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


