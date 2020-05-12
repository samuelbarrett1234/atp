/**
\file

\author Samuel Barrett

*/


#include <numeric>
#include <vector>
#include "ATPStatsMinAssignment.h"


namespace ublas = boost::numeric::ublas;


namespace atp
{
namespace stats
{


void minimum_assignment_recursive(ublas::matrix<float>& distances,
	std::vector<bool>& in_use, size_t j, float cur_cost,
	float& best_cost_ever)
{
	if (j == distances.size2())
	{
		// no more left
		best_cost_ever = std::min(best_cost_ever,
			cur_cost);

		return;
	}

	for (size_t i = 0; i < in_use.size(); ++i)
	{
		if (!in_use[i])
		{
			in_use[i] = true;

			const float edge_dist = distances(i, j);

			minimum_assignment_recursive(distances,
				in_use, j + 1, cur_cost + edge_dist,
				best_cost_ever);

			in_use[i] = false;
		}
	}
}


float minimum_assignment(ublas::matrix<float>& distances)
{
	const size_t N = distances.size1();
	const size_t M = distances.size2();

	ATP_STATS_PRECOND(N >= M);
	ATP_STATS_PRECOND(M >= 1);

	float best_cost = std::numeric_limits<float>::max();
	std::vector<bool> in_use;
	in_use.resize(N, false);

	minimum_assignment_recursive(distances,
		in_use, 0, 0.0f, best_cost);

	return best_cost;
}


}  // namespace stats
}  // namespace atp


