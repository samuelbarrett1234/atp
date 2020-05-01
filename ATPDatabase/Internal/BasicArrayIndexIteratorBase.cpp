/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndexIteratorBase.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


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


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


