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
	return DArray::to_stmt_arr(d_arr);
}


}  // namespace db
}  // namespace atp


