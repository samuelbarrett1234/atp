#pragma once

/**
\file

\author Samuel Barrett

\brief Contains a select iterator for the basic array index which
	performs no filtering (hence "all" in the name).

*/


#include <vector>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"
#include "../Interfaces/DBIterators.h"
#include "BasicArrayIndexIteratorBase.h"


namespace atp
{
namespace db
{


class BasicArrayIndex;  // forward declaration


namespace basic_array_index_detail
{


/**
\brief Implementation of the IDBSelectIterator for BasicArrayIndex.

\details Also used to do some of the heavy lifting for update/delete
	iterators. Note that this iterator has no support for row
	predicates (only selecting some of the rows), hence you would
	need to create a wrapper iterator to do that for you.
*/
class ATP_DATABASE_API SelectAllIterator :
	public IteratorBase,
	public IDBSelectIterator
{
public:
	SelectAllIterator(const logic::ILanguage& lang,
		const logic::IModelContext& ctx,
		const BasicArrayIndex& parent,
		std::shared_ptr<ILock> p_lock,
		std::shared_ptr<IReadableStream> p_stream,
		const std::vector<DType>& col_types,
		const std::vector<std::string>& col_names);

	void advance() override;
	bool valid() const override;

	inline ColumnList cols() const override
	{
		return m_parent.cols();
	}
	size_t num_cols() const override
	{
		return m_parent.num_cols();
	}
	DType type(const Column& col) const override;
	DValue get(const Column& col) const override;
	inline std::vector<DValue> get_all() const override
	{
		return m_row;
	}

private:
	/**
	\brief Read the row data at `m_cur_idx_in_block`

	\warning Does not check the row predicate `m_row_pred`
	*/
	void readrow();

protected:
	// NOTE: the SelectAllIterator imposes NO invariant on the current
	// input/output stream positions!

	// for creating statements
	const logic::ILanguage& m_lang;
	const logic::IModelContext& m_ctx;

	const BasicArrayIndex& m_parent;

	// the currently-read row, which would be placed at index
	// m_cur_idx_in_block
	std::vector<DValue> m_row;

	// index of the next row to read from the block
	size_t m_cur_idx_in_block;

	// the file offset of the end of the current block
	size_t m_cur_block_end_off;

	// the file offset of the start of the next block
	size_t m_next_block_off;
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


