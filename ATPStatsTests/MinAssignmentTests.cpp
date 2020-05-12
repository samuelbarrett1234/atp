/**
\file

\author Samuel Barrett

\brief Test suite for the "minimum assignment" function, for
	computing the least cost maximal matching in a complete bipartite
	graph.

*/


#include <ATPStatsMinAssignment.h>
#include "Test.h"


using atp::stats::minimum_assignment;
namespace ublas = boost::numeric::ublas;


BOOST_AUTO_TEST_SUITE(MinAssignmentTests);


BOOST_AUTO_TEST_CASE(minimum_assignment_square_test)
{
	ublas::matrix<float> dists(3, 3);

	dists(0, 0) = 33.0f; dists(0, 1) = 1.0f; dists(0, 2) = 10.0f;
	dists(1, 0) = 1.0f; dists(1, 1) = 2.0f; dists(1, 2) = 5.0f;
	dists(2, 0) = 10.0f; dists(2, 1) = 5.0f; dists(2, 2) = 6.0f;

	BOOST_TEST(minimum_assignment(dists) == 8.0f);
}


BOOST_AUTO_TEST_CASE(minimum_assignment_not_square_test)
{
	ublas::matrix<float> dists(3, 2);

	dists(0, 0) = 33.0f; dists(0, 1) = 1.0f;
	dists(1, 0) = 1.0f; dists(1, 1) = 2.0f;
	dists(2, 0) = 10.0f; dists(2, 1) = 5.0f;

	BOOST_TEST(minimum_assignment(dists) == 2.0f);
}


BOOST_AUTO_TEST_SUITE_END();


