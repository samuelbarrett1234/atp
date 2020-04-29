/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndex.h"
#include "BasicArrayIndexIterators.h"
#include "../Interfaces/IBufferManager.h"
#include "../Interfaces/ILockManager.h"


namespace atp
{
namespace db
{


BasicArrayIndex::BasicArrayIndex(IBufferManager& buf_mgr,
	ILockManager& lk_mgr, ResourceName my_name,
	const std::vector<std::string>& col_names,
	const std::vector<DType>& col_types) :
	m_buf_mgr(buf_mgr), m_name(my_name), m_col_names(col_names),
	m_col_types(col_types), m_lock_mgr(lk_mgr)
{
}


QuerySupport BasicArrayIndex::get_support(
	const DBContainerQuery& q) const
{
	if (q.traversal_mode() !=
		DBContainerQuery::TraversalMode::DEFAULT)
		return QuerySupport::NONE;

	// check that, if they have a lock, it has locked the resource
	// we need! (if they don't provide a lock then we will get our
	// own later)
	if (q.lock() != nullptr)
	{
		const auto res_list = q.lock()->get_locked_resources();

		auto iter = std::find(res_list.begin(), res_list.end(),
			m_name);

		if (iter == res_list.end())
			return QuerySupport::NONE;
	}

	// if inserting, check column types to make sure they're valid
	if (q.query_kind() == DBContainerQuery::QueryKind::INSERT)
	{
		if (q.is_inserting_many_rows())
		{
			const auto& rows = q.rows();

			if (rows.size() != m_col_types.size())
				return QuerySupport::NONE;

			for (size_t i = 0; i < m_col_types.size(); ++i)
			{
				if (rows[i].type() != m_col_types[i])
					return QuerySupport::NONE;
			}
		}
		else
		{
			const auto& row = q.row();

			if (row.size() != m_col_types.size())
				return QuerySupport::NONE;

			for (size_t i = 0; i < m_col_types.size(); ++i)
			{
				if (row[i].type() != m_col_types[i])
					return QuerySupport::NONE;
			}
		}
	}

	// this operation should be optimised on ANY index, since it's so
	// trivial
	if (q.where_kind() == DBContainerQuery::WhereKind::FIND_ALL)
		return QuerySupport::OPTIMISED;

	// this index doesn't try to do anything well, except just
	// iterating over arbitrarily
	return QuerySupport::POOR;
}


std::shared_ptr<IDBIterator> BasicArrayIndex::begin_query(
	const DBContainerQuery& q)
{
	ATP_DATABASE_PRECOND(get_support(q) !=
		QuerySupport::NONE);

	// the query is read only iff it is a SELECT
	const bool read_only = (q.query_kind() ==
		DBContainerQuery::QueryKind::SELECT);

	std::shared_ptr<ILock> p_lock = q.lock();
	if (p_lock == nullptr)
	{
		// try to obtain a lock, as they haven't:

		ResourceList res_list;
		res_list.push_back(m_name);

		if (read_only)
		{
			p_lock = m_lock_mgr.request_read_access(res_list);
		}
		else
		{
			p_lock = m_lock_mgr.request_write_access(res_list);
		}
	}

	// if they didn't provide a lock, and we weren't able to obtain
	// one for ourselves, we cannot carry out the operation:
	if (p_lock == nullptr)
		return nullptr;

	// obtain the memory stream
	std::shared_ptr<IReadableStream> p_stream;
	if (read_only)
	{
		p_stream = m_buf_mgr.request_read_access(m_name);
	}
	else
	{
		p_stream = m_buf_mgr.request_write_access(m_name);
	}

	// for some reason we weren't able to get access to the stream
	// to perform the operation:
	if (p_stream == nullptr)
		return nullptr;

	// else we are good to go; construct the right iterator:
	switch (q.query_kind())
	{

	default:
		ATP_DATABASE_ASSERT(false && "bad query kind!");
		return nullptr;
	}
}


bool BasicArrayIndex::has_col_flag(
	ColumnFlag cf, const Column& col) const
{
	ATP_DATABASE_PRECOND(cols().contains(col));

	const size_t idx = cols().index_of(col);

	switch (cf)
	{
	case ColumnFlag::AUTO_KEY:
		return (m_autokey_col.has_value() &&
			*m_autokey_col == idx);

	default:
		return false;
	}
}


}  // namespace db
}  // namespace atp


