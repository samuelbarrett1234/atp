/**
\file

\author Samuel Barrett

\brief Test suite for the `BasicStoppingStrategy`

*/


#include <random>
#include <Internal/BasicStoppingStrategy.h>
#include "Test.h"


using atp::search::BasicStoppingStrategy;


struct BasicStoppingStrategyTestsFixture
{
	// hyperparameters
	const float lambda = 1.0e1f,
		alpha = 0.05f;
	const size_t N = 3;

	BasicStoppingStrategy strat;

	BasicStoppingStrategyTestsFixture() :
		strat(N, lambda, alpha)
	{ }
};


BOOST_AUTO_TEST_SUITE(StoppingStrategyTests);
BOOST_FIXTURE_TEST_SUITE(BasicStoppingStrategyTests,
	BasicStoppingStrategyTestsFixture);


BOOST_AUTO_TEST_CASE(starts_not_stopped)
{
	BOOST_TEST(!strat.is_stopped());
}


// repeat this test a few times
BOOST_DATA_TEST_CASE(test_initial_fill,
	boost::unit_test::data::xrange(10),
	repeat_index)
{
	// generate random data
	std::random_device dev;
	std::uniform_real_distribution<float>
		cost_dist(0.1f, 10.0f),
		benefit_dist(-1.0f, 1.0f);

	for (size_t i = 0; i < N; ++i)
	{
		BOOST_TEST(!strat.is_stopped());
		strat.add(benefit_dist(dev),
			cost_dist(dev));
	}
}


BOOST_AUTO_TEST_CASE(test_not_stopped,
	* boost::unit_test_framework::depends_on(
		"StoppingStrategyTests/BasicStoppingStrategyTests/test_initial_fill"))
{
	strat.add(1.0f, 0.1f);
	strat.add(2.0f, 0.15f);
	strat.add(3.0f, 0.09f);

	BOOST_TEST(!strat.is_stopped());
}


BOOST_AUTO_TEST_CASE(test_stopped_after_costly_data,
	*boost::unit_test_framework::depends_on(
		"StoppingStrategyTests/BasicStoppingStrategyTests/test_not_stopped"))
{
	strat.add(1.0f, 0.1f);
	strat.add(2.0f, 0.15f);
	strat.add(3.0f, 0.09f);
	strat.add(2.0f, 6.0f);

	BOOST_TEST(strat.is_stopped());
}


BOOST_AUTO_TEST_CASE(test_stopped_after_big_input)
{
	strat.add(1.0f, 0.1f);
	strat.add(2.0f, 0.15f);
	strat.add(3.0f, 0.09f);
	strat.add(2.0f, 0.1f);
	strat.add(1.0e6f, 0.2f);

	BOOST_TEST(strat.is_stopped());
}


BOOST_AUTO_TEST_CASE(test_not_stopped_after_big_input_removed,
	* boost::unit_test_framework::depends_on(
		"StoppingStrategyTests/BasicStoppingStrategyTests/test_stopped_after_big_input"))
{
	strat.add(1.0f, 0.1f);
	strat.add(1.2f, 0.2f);
	strat.add(1.19f, 0.3f);
	strat.add(0.5f, 0.1f);
	strat.add(1.0e6f, 0.2f);

	strat.max_removed();

	BOOST_TEST(!strat.is_stopped());
}


BOOST_DATA_TEST_CASE(test_resistant_to_no_variance,
	boost::unit_test::data::random(
		boost::unit_test::data::distribution =
		std::uniform_real_distribution<float>(-2.0f, 2.0f)) ^
	boost::unit_test::data::random(
		boost::unit_test::data::distribution = 
		std::uniform_real_distribution<float>(0.01f, 10.0f)) ^
	boost::unit_test::data::xrange(50),
	var, cost, idx)
{
	for (size_t i = 0; i < N; ++i)
		strat.add(var, cost);

	BOOST_TEST(strat.is_stopped());
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


