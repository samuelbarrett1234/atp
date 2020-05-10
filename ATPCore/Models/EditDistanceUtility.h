#pragma once


/**
\file

\author Samuel Barrett

*/


#include <map>
#include <boost/numeric/ublas/matrix.hpp>
#include <ATPLogic.h>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
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


/**
\brief Given a matrix of shape (N,M) where N >= M find a sequence of
	M integers x_i in the range 0..N-1 such that the sum of distances
	(x_i, i) is minimised.

\details Given a pairwise distances matrix, this computes a good
	subset and permutation of the elements in this matrix which
	minimises the sum cost. This is equivalent to finding the minimum
	cost matching.

\note The size of these matrices is bounded (i.e. O(1)) when given a
	fixed model context, so this algorithm is technically constant
	time w.r.t. the size of the statements we're computing the edit
	distance for.

\pre distances.size1() >= distances.size2()

\returns The minimum cost, as described above.
*/
ATP_CORE_API float minimum_assignment(
	boost::numeric::ublas::matrix<float>& distances);


/**
\brief Compute the edit distance between two logical statements

\param sub_costs The cost of each symbol substitution.

\pre See requirements about the EditDistSubCosts mapping.
*/
ATP_CORE_API float edit_distance(
	const logic::IStatement& stmt1,
	const logic::IStatement& stmt2,
	const EditDistSubCosts& sub_costs);


/**
\brief Compute the edit distance for each pair in the cross product
	of the two arrays.

\details This is strictly more efficient than calling edit_distance
	on each pair.

\pre See requirements about the EditDistSubCosts mapping.

\returns An array A such that A[i][j] = edit_distance(stmtarr1.at(i),
	stmtarr2.at(j))
*/
ATP_CORE_API std::vector<std::vector<float>> pairwise_edit_distance(
	const logic::IStatementArray& stmtarr1,
	const logic::IStatementArray& stmtarr2,
	const EditDistSubCosts& sub_costs);


}  // namespace core
}  // namespace atp


