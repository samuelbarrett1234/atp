#pragma once

/**
\file

\author Samuel Barrett

\brief Contains the insert iterator for the BasicArrayIndex index.

*/


#include <vector>
#include <sstream>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"
#include "../Interfaces/DBIterators.h"
#include "BasicArrayIndexIteratorBase.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


/**
\brief Implementation of IDBInsertIterator for the BasicArrayIndex.

\details Works by immediately seeking to the last block of the file
	and then adding the rows to the end, provided the block has
	enough capacity, and creating new blocks where necessary.
*/
class ATP_DATABASE_API InsertIterator :
	public IteratorBase,
	public IDBInsertIterator
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
	std::vector<std::shared_ptr<
		std::stringstream>> m_col_mem_streams;
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


