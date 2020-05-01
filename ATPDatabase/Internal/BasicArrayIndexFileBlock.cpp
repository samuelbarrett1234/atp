/**
\file

\author Samuel Barrett

*/


#include "BasicArrayIndexFileBlock.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


size_t fit_rows(size_t excess_capacity,
	const std::vector<size_t>& off_array,
	size_t next_row_to_insert)
{
	// the amount of memory inserted so far
	const size_t mem_used = (next_row_to_insert > 0) ?
		off_array[next_row_to_insert - 1] : 0;

	auto iter = std::lower_bound(off_array.begin(),
		off_array.end(), mem_used + excess_capacity);

	// CHECK!
	// This is almost certainly not correct :)

	ATP_DATABASE_ASSERT(iter == off_array.end() ||
		*iter - mem_used <= excess_capacity);

	return std::distance(off_array.begin(), iter);
}


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


