#pragma once

/**
\file

\author Samuel Barrett

\brief Contains a base iterator class, used by all of the other basic
	array index iterators.

*/


#include <vector>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"
#include "../Interfaces/DBIterators.h"
#include "../Interfaces/IBufferManager.h"
#include "BasicArrayIndexFileBlock.h"


namespace atp
{
namespace db
{


class ILock;  // forward declaration


namespace basic_array_index_detail
{


/**
\brief A base iterator class for all iterators from this index
	type.
*/
class ATP_DATABASE_API IteratorBase :
	public IDBIterator
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
	const std::vector<DType>& m_col_types;
	const std::vector<std::string>& m_col_names;

private:
	std::shared_ptr<ILock> m_lock;
	std::shared_ptr<IReadableStream> m_stream;
	IReadWriteStream* m_rw_stream;  // may be null
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


