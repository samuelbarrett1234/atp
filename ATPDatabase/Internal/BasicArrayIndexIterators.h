#pragma once

/**
\file

\author Samuel Barrett

\brief Contains all of the iterators for the BasicArrayIndex index.

*/


#include <vector>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "../ATPDatabaseAPI.h"
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


struct ATP_DATABASE_API Block
{
	void read(std::istream& is);
	void write(std::ostream& os);

	// number of bytes represented by each column
	std::vector<size_t> cur_col_sizes;

	// max num bytes each column can occupy in this block
	std::vector<size_t> col_total_capacities;

	// num rows currently stored in this block
	size_t block_row_count;

	// the number of bytes one would have to advance the stream
	// from the end of this block to the start of the next block.
	// this is of course equal to the total byte capacity size of
	// the block.
	size_t offset_of_next_block;
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
		std::shared_ptr<IReadableStream> p_stream);

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
		const std::vector<DArray>& rows);

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
	*/
	void serialise_insert_data(const std::vector<DArray>& rows);

private:
	// here we will serialise the data for each column into a
	// single memory stream
	std::vector<std::stringstream> m_col_mem_streams;
	// here we will record the offset of the END of each row
	// element in bytes, for each column.
	std::vector<std::vector<size_t>> m_col_element_offs;

	// the index of the next row to insert
	size_t m_input_row_idx;

	const size_t m_num_rows_to_insert, m_num_cols;

	// the offset of the start of the last block in the file.
	size_t m_end_block_offset;

	// example: if we were to read the bytes from 0 to
	// m_col_element_offs[5] from the stream m_col_mem_streams[5]
	// then this would have read elements 0..5 inclusive.
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


