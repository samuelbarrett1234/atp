/**
\file

\author Samuel Barrett

\brief Test suite for the Hidden Markov Model utility functions
	located in "HMMUtility.h"

*/


#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <Models/HMMUtility.h>
#include "Test.h"


namespace hmm = atp::core::hmm;
namespace ublas = boost::numeric::ublas;


struct HMMUtilityTestsFixture
{
	HMMUtilityTestsFixture() :
		st_trans(2, 2),
		st_obs(2, 2),
		initial_state(2)
	{
		initial_state(0) = 0.5f;
		initial_state(1) = 0.5f;

		st_trans(0, 0) = 0.6f;
		st_trans(0, 1) = 0.4f;
		st_trans(1, 0) = 0.0f;
		st_trans(1, 1) = 1.0f;

		st_obs(0, 0) = 0.8f;
		st_obs(0, 1) = 0.2f;
		st_obs(1, 0) = 0.3f;
		st_obs(1, 1) = 0.7f;
	}

	ublas::vector<float> initial_state;
	ublas::matrix<float> st_trans, st_obs;
};


BOOST_FIXTURE_TEST_SUITE(HMMUtilityTests,
	HMMUtilityTestsFixture);


BOOST_AUTO_TEST_CASE(test_forward_algorithm,
	*boost::unit_test::tolerance(1.0e-6f))
{
	std::vector<size_t> obs_seq;
	obs_seq.push_back(0);
	obs_seq.push_back(0);
	obs_seq.push_back(1);

	auto result = hmm::forward(initial_state,
		st_trans, st_obs, obs_seq);

	BOOST_REQUIRE(result.size1() == 4);
	BOOST_REQUIRE(result.size2() == 2);

	// initial states should be uniformly distributed
	BOOST_TEST(result(0, 0) == 0.5f);
	BOOST_TEST(result(0, 1) == 0.5f);

	// example obtained from:
	// http://www.cs.tut.fi/kurssit/SGN-24006/PDF/L08-HMMs.pdf
	// but changed initial state distribution to (0.5 0.5)

	BOOST_TEST(result(1, 0) == 0.24f);
	BOOST_TEST(result(1, 1) == 0.21f);

	BOOST_TEST(result(2, 0) == 0.1152f);
	BOOST_TEST(result(2, 1) == 0.0918f);

	BOOST_TEST(result(3, 0) == 0.013824f);
	BOOST_TEST(result(3, 1) == 0.096516f);
}


BOOST_AUTO_TEST_CASE(test_backward_algorithm,
	*boost::unit_test::tolerance(1.0e-6f))
{
	std::vector<size_t> obs_seq;
	obs_seq.push_back(0);
	obs_seq.push_back(0);
	obs_seq.push_back(1);

	auto result = hmm::backward(st_trans, st_obs, obs_seq);

	BOOST_REQUIRE(result.size1() == 4);
	BOOST_REQUIRE(result.size2() == 2);

	BOOST_TEST(result(0, 0) == 0.15768f);
	BOOST_TEST(result(0, 1) == 0.063f);

	// example obtained from:
	// http://www.cs.tut.fi/kurssit/SGN-24006/PDF/L08-HMMs.pdf

	BOOST_TEST(result(1, 0) == 0.276f);
	BOOST_TEST(result(1, 1) == 0.21f);

	BOOST_TEST(result(2, 0) == 0.4f);
	BOOST_TEST(result(2, 1) == 0.7f);

	// end values are always set to 1
	BOOST_TEST(result(3, 0) == 1.0f);
	BOOST_TEST(result(3, 1) == 1.0f);
}


BOOST_AUTO_TEST_SUITE_END();


