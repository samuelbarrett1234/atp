/**
\file

\author Samuel Barrett

\brief This suite tests the `FixedStoppingStrategy`
*/


#include <Internal/FixedStoppingStrategy.h>
#include "Test.h"


using atp::search::FixedStoppingStrategy;


BOOST_AUTO_TEST_SUITE(FixedStoppingStrategyTests);


BOOST_AUTO_TEST_CASE(basic_test)
{
	const size_t N = 10;

	FixedStoppingStrategy strat(N);

	for (size_t i = 0; i < N; ++i)
	{
		// this will also test that repeated benefits is allowed
		BOOST_TEST(!strat.is_stopped());
		strat.add(1.0f, 1.0f);
	}
	BOOST_TEST(strat.is_stopped());

	strat.max_removed();
	BOOST_TEST(!strat.is_stopped());

	strat.add(2.0f, 2.0f);
	BOOST_TEST(strat.is_stopped());
}


BOOST_AUTO_TEST_SUITE_END();


