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
	std::shared_ptr<IReadableStream> p_stream,
	const std::vector<DType>& col_types,
	const std::vector<std::string>& col_names) :
	m_lock(std::move(p_lock)),
	m_stream(std::move(p_stream)),
	m_rw_stream(dynamic_cast<IReadWriteStream*>(m_stream.get())),
	m_col_types(col_types), m_col_names(col_names),
	m_cur_block(col_types)
{
	// only if nonempty file
	if (is().good())
		m_cur_block.read(is());
}


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


SelectAllIterator::SelectAllIterator(const logic::ILanguage& lang,
	const logic::IModelContext& ctx,
	const BasicArrayIndex& parent, std::shared_ptr<ILock> p_lock,
	std::shared_ptr<IReadableStream> p_stream,
	const std::vector<DType>& col_types,
	const std::vector<std::string>& col_names) :
	IteratorBase(std::move(p_lock), std::move(p_stream),
		col_types, col_names), m_lang(lang), m_ctx(ctx),
	m_parent(parent), m_cur_idx_in_block(0)
{
	// the IteratorBase has already read the first block for us, so
	// we should've been left pointing to the end of the block
	m_cur_block_end_off = is().tellg();

	m_next_block_off = m_cur_block_end_off +
		m_cur_block.next_block_offset();

	// if file is nonempty...
	if (m_cur_idx_in_block < m_cur_block.num_rows())
	{
		// read first row
		readrow();
	}
}


void SelectAllIterator::advance()
{
	ATP_DATABASE_PRECOND(valid());
	
	// WARNING: be wary of empty blocks in the middle of the file!

	do
	{
		// go to next row in the block
		++m_cur_idx_in_block;

		// if we have reached the end of the block...
		if (m_cur_idx_in_block == m_cur_block.num_rows())
		{
			// if there is not another block, then enter invalid
			// state, else get next block and reset current index to
			// 0. Then call readrow().

			// go to start of next block
			m_cur_idx_in_block = 0;

			// seek to where we would find the next block, if it
			// exists
			is().seekg(m_next_block_off, std::ios::beg);

			// is there any data left? we may have reached the end of
			// the file.
			if (is().good())
			{
				// read the block information
				m_cur_block.read(is());

				// get our bearings
				m_cur_block_end_off = is().tellg();
				m_next_block_off = m_cur_block_end_off +
					m_cur_block.next_block_offset();

				// get the data if the block is nonempty
				// (if the block is empty but the file is good, it
				// indicates an empty block in the middle of the file
				// in which case we must advance to the next block
				// instead to keep going).
				if (valid())
					readrow();
			}
			else
			{
				// clear the current block
				m_cur_block = Block(m_col_types);

				ATP_DATABASE_ASSERT(!valid());

				return;  // we are done here
			}
		}
		else
		{
			// we've not needed to advance to the next block and
			// we just need to read the current row's data
			readrow();
		}

	} while (!valid());

	// if we reach here, the iterator is valid() and
	// thus we are ready to proceed.
}


bool SelectAllIterator::valid() const
{
	return m_cur_idx_in_block < m_cur_block.num_rows();
}


DType SelectAllIterator::type(const Column& col) const
{
	if (col.has_index())
	{
		const size_t idx = col.index();

		ATP_DATABASE_PRECOND(idx < m_col_types.size());

		return m_col_types[idx];
	}
	else
	{
		auto col_list = cols();

		ATP_DATABASE_PRECOND(col_list.contains(col));

		const size_t idx = col_list.index_of(col);

		ATP_DATABASE_ASSERT(idx < m_col_types.size());

		return m_col_types[idx];
	}
}


DValue SelectAllIterator::get(const Column& col) const
{
	if (col.has_index())
	{
		const size_t idx = col.index();

		ATP_DATABASE_PRECOND(idx < m_col_types.size());

		return m_row[idx];
	}
	else
	{
		auto col_list = cols();

		ATP_DATABASE_PRECOND(col_list.contains(col));

		const size_t idx = col_list.index_of(col);

		ATP_DATABASE_ASSERT(idx < m_col_types.size());

		return m_row[idx];
	}
}


void SelectAllIterator::readrow()
{
	ATP_DATABASE_ASSERT(m_cur_idx_in_block < m_cur_block.num_rows());

	m_row.clear();
	m_row.reserve(m_col_types.size());

	for (size_t i = 0; i < m_col_types.size(); ++i)
	{
		const size_t elem_off = m_cur_block.col_elem_offset(i,
			m_cur_idx_in_block);

		const size_t elem_pos = m_cur_block_end_off + elem_off;

		is().seekg(elem_pos, std::ios::beg);

		m_row.emplace_back(DValue::load(is(), m_col_types[i], m_lang,
			m_ctx));
	}
}


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


