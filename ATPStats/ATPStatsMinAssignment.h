#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a function for computing a minimum cost maximal
	matching in a complete bipartite graph.

*/


#include <boost/numeric/ublas/matrix.hpp>
#include <ATPLogic.h>
#include "ATPStatsAPI.h"


namespace atp
{
namespace stats
{


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
ATP_STATS_API float minimum_assignment(
	boost::numeric::ublas::matrix<float>& distances);


}  // namespace stats
}  // namespace atp


