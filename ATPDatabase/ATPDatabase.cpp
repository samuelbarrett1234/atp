/**
\file

\author Samuel Barrett

*/


#include "ATPDatabase.h"
#include <vector>


namespace atp
{
namespace db
{


logic::StatementArrayPtr db_arr_to_stmt_arr(
	const DArray& d_arr)
{
	ATP_DATABASE_PRECOND(d_arr.type() == DType::STMT);

	// bit of an inefficient function, but hopefully shouldn't be
	// called all that often

	std::vector<logic::StatementArrayPtr> singletons;
	singletons.reserve(d_arr.size());

	for (size_t i = 0; i < d_arr.size(); ++i)
	{
		auto p_stmt = d_arr.val_at(i).as_stmt();
		singletons.emplace_back(
			logic::from_statement(*p_stmt));
	}

	return logic::concat(singletons);
}


}  // namespace db
}  // namespace atp


