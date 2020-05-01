#pragma once

/**
\file

\author Samuel Barrett

\brief Contains a data structure used in the file structure of the
	basic array index.

*/


#include <vector>
#include <ostream>
#include <istream>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


/**
\brief Get the number of rows that would fit into the excess capacity

\param excess_capacity The max number of bytes that we are allowed to
	read.

\param off_array off_array[i] represents the number of bytes needed
	to store elements 0..i inclusive.

\param next_row_to_insert The index of the next row that needs to be
	inserted.
*/
ATP_DATABASE_API size_t fit_rows(size_t excess_capacity,
	const std::vector<size_t>& off_array,
	size_t next_row_to_insert);


class ATP_DATABASE_API Block
{
public:
	/**
	\brief Get the total number of bytes reserved for all columns for
		a block with the given data types.
	*/
	static size_t data_space(
		const std::vector<DType>& col_types);

	/**
	\brief Create an empty block (or rather, a valid block that
		thinks it is empty). This constructor will be enough to write
		out a new block.

	\param col_types This not only tells the block how many columns
		there are, but also the column datatypes tells the block how
		much space to reserve for each column.
	*/
	Block(const std::vector<DType>& col_types);

	/**
	\brief Read (only) block information from the stream into this
		object, overwriting any existing data.

	\param is The input stream should be pointing to the beginning
		of the block.
	*/
	void read(std::istream& is);

	/**
	\brief Write (only) block information from this object into the
		stream.

	\param os The output stream should be pointing to the beginning
		of the block section.
	*/
	void write(std::ostream& os) const;

	/**
	\brief Write a bunch of blank memory to the output stream, but in
		doing so, reserving enough space for this block's data. This
		will then allow you to seek to the relevant block data
		positions etc.

	\param os The output stream should be pointing to the end of this
		block object's memory.

	\note Will reserve exactly the amount of space returned by
		`data_space`.

	\see data_space
	*/
	void reserve_space(std::ostream& os) const;

	/**
	\brief The byte offset (from the end of the current block) of the
		start of the given column.
	*/
	size_t col_start_offset(size_t col_idx) const;

	/**
	\brief The byte offset of the start of the given element in the
		given column.
	*/
	size_t col_elem_offset(size_t col_idx, size_t row_idx) const;

	/**
	\brief The byte offset of the start of the next block from the
		end of the current block.
	*/
	inline size_t next_block_offset() const
	{
		return m_offset_of_next_block;
	}

	/**
	\brief The number of bytes being used for a particular column.
	*/
	inline size_t column_current_size(size_t col_idx) const
	{
		ATP_DATABASE_PRECOND(col_idx < m_cur_col_sizes.size());
		return m_cur_col_sizes[col_idx];
	}

	/**
	\brief The total number of bytes allocated for a particular
		column.
	*/
	inline size_t column_total_size(size_t col_idx) const
	{
		ATP_DATABASE_PRECOND(col_idx <
			m_col_total_capacities.size());
		return m_col_total_capacities[col_idx];
	}

	/**
	\brief The total size minus the current size
	*/
	inline size_t column_excess_capacity(size_t col_idx) const
	{
		ATP_DATABASE_PRECOND(col_idx < m_cur_col_sizes.size());
		ATP_DATABASE_ASSERT(m_cur_col_sizes.size()
			== m_col_total_capacities.size());

		return m_col_total_capacities[col_idx]
			- m_cur_col_sizes[col_idx];
	}

	/**
	\brief The number of rows stored.
	*/
	inline size_t num_rows() const
	{
		return m_block_row_count;
	}

	/**
	\brief The total number of bytes reserved after the end of the
		block for data.

	\note Equals the value returned by `data_space`, but avoids
		recomputing, as this value is cached.

	\see data_space
	*/
	inline size_t total_space() const
	{
		return m_total_space;
	}

	/**
	\brief Add to the row count.

	\warning After modifying this object's state, you need to
		remember to save it to the file again!
	*/
	inline void add_rows(size_t num_rows)
	{
		m_block_row_count += num_rows;
	}

private:
	// number of bytes represented by each column
	std::vector<size_t> m_cur_col_sizes;

	// max num bytes each column can occupy in this block
	std::vector<size_t> m_col_total_capacities;

	// num rows currently stored in this block
	size_t m_block_row_count;

	// the number of bytes one would have to advance the stream
	// from the end of this block to the start of the next block.
	// this is of course equal to the total byte capacity size of
	// the block.
	size_t m_offset_of_next_block;

	// the total number of bytes reserved for data (this is exactly
	// equal to `data_space`, however is cached here to avoid
	// recomputing.
	size_t m_total_space;
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


