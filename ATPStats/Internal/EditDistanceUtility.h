#pragma once


/**
\file

\author Samuel Barrett

\brief Contains definitions used for all edit distance files.

*/


#include <map>
#include "../ATPStatsAPI.h"


namespace atp
{
namespace stats
{


/**
\brief Used for storing the costs of substituting two symbols in the
	edit distance algorithm

\details A mapping from (symbol ID 1, symbol ID 2) to cost, where the
	cost represents the cost of substituting 1 -> 2

\invariant The arity of symbol 1 is at least the arity of symbol 2
	(this is because substitutions which increase the arity are kind
	of ambiguous - what would the distances of the new arguments be?)

\invariant This mapping must contain, for EVERY pair of symbols (a,b)
	such that b's arity <= a's arity, a floating point value. Note
	that for consistent results, you should set (a,a) to have cost 0
	for every symbol a.
*/
typedef std::map<std::pair<size_t, size_t>, float> EditDistSubCosts;


}  // namespace stats
}  // namespace atp


