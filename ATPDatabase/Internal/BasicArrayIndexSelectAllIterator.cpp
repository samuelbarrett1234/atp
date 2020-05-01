/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndexSelectAllIterator.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


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


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


