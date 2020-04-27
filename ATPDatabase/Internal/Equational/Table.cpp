#include "Table.h"


namespace atp
{
namespace db
{
namespace equational
{


bool Table::has_col_flag(ColumnFlag cf, const Column& col) const
{
	ATP_DATABASE_PRECOND(cols().contains(col));

	const size_t idx = cols().index_of(col);

	switch (cf)
	{
	case ColumnFlag::UNIQUE:
		ATP_DATABASE_ASSERT(idx < m_col_unique.size());
		return m_col_unique[idx];

	case ColumnFlag::AUTO_KEY:
		return (m_autokey_col.has_value() &&
			*m_autokey_col == idx);

	default:
		return false;
	}
}


}  // namespace equational
}  // namespace db
}  // namespace atp


