#pragma once

/**
\file

\author Samuel Barrett

\brief Contains all of the iterators for the BasicArrayIndex index.

*/


#include <vector>
#include <sstream>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"
#include "../Interfaces/DBIterators.h"
#include "../Interfaces/IBufferManager.h"


namespace atp
{
namespace db
{


class ILock;  // forward declaration


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
	size_t col_offset(size_t col_idx) const;

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



/**
\brief A base iterator class for all iterators from this index
	type.
*/
class ATP_DATABASE_API IteratorBase :
	public virtual IDBIterator
{
public:
	IteratorBase(std::shared_ptr<ILock> p_lock,
		std::shared_ptr<IReadableStream> p_stream,
		const std::vector<DType>& col_types,
		const std::vector<std::string>& col_names);

protected:
	inline bool writable() const
	{
		return (m_rw_stream != nullptr);
	}
	inline std::istream& is()
	{
		return m_stream->is();
	}
	inline std::ostream& os()
	{
		ATP_DATABASE_PRECOND(writable());
		return m_rw_stream->os();
	}

protected:
	Block m_cur_block;
	size_t m_cur_idx_in_block;
	const std::vector<DType>& m_col_types;
	const std::vector<std::string>& m_col_names;

private:
	std::shared_ptr<ILock> m_lock;
	std::shared_ptr<IReadableStream> m_stream;
	IReadWriteStream* m_rw_stream;  // may be null
};


class ATP_DATABASE_API InsertIterator :
	public virtual IteratorBase,
	public virtual IDBInsertIterator
{
public:
	InsertIterator(std::shared_ptr<ILock> p_lock,
		std::shared_ptr<IReadableStream> p_stream,
		const std::vector<DArray>& rows,
		const std::vector<DType>& col_types,
		const std::vector<std::string>& col_names);

	void advance() override;
	bool valid() const override;
	void insert_advance() override;

private:
	/**
	\brief Go right to the end of the stream, so we can insert
		into the end block.
	*/
	void bring_to_end();

	/**
	\brief Turn the input array into a byte stream for each column.

	\details See m_col_mem_streams and m_col_element_offs

	\note This is only called once, in the constructor.
	*/
	void serialise_insert_data(const std::vector<DArray>& rows);

	/**
	\brief Compute the number of rows that can fit in the current
		block.
	*/
	size_t num_rows_that_can_fit() const;
private:
	// here we will serialise the data for each column into a
	// single memory stream
	std::vector<std::stringstream> m_col_mem_streams;
	// here we will record the offset of the END of each row
	// element in bytes, for each column.
	std::vector<std::vector<size_t>> m_col_element_offs;

	// example: if we were to read the bytes from 0 to
	// m_col_element_offs[5] from the stream m_col_mem_streams[5]
	// then this would have read elements 0..5 inclusive.

	// the index of the next row to insert
	size_t m_input_row_idx;

	const size_t m_num_rows_to_insert, m_num_cols;

	// INVARIANT: the output stream always points to the end of the
	// current block.
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


