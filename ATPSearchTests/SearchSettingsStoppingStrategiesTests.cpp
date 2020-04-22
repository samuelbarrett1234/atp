/*
\file

\author Samuel Barrett

\brief Test suite for stopping strategy parsing in search settings
	files.

*/


#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Internal/SearchSettingsStoppingStrategies.h>
#include <Internal/IteratorManager.h>
#include "Test.h"


using atp::search::try_load_stopping_strategies;
using atp::search::try_load_fixed_stopping_strategy;
using atp::search::try_load_basic_stopping_strategy;
using atp::search::IteratorManager;
using boost::property_tree::ptree;
using boost::property_tree::read_json;


struct SearchSettingsStoppingStrategiesTestsFixture
{
	std::unique_ptr<IteratorManager> p_iter_mgr =
		std::make_unique<IteratorManager>();

	std::stringstream s;
};


BOOST_FIXTURE_TEST_SUITE(SearchSettingsStoppingStrategiesTests,
	SearchSettingsStoppingStrategiesTestsFixture);


BOOST_AUTO_TEST_CASE(test_bad_strat_type)
{
	s << "{ \"type\" : \"bad-strat-type\" }";

	ptree pt;
	read_json(s, pt);

	BOOST_TEST(!try_load_stopping_strategies(
		*p_iter_mgr, pt));
}


BOOST_DATA_TEST_CASE(test_fixed_stopping_strategy,
	boost::unit_test::data::make({ 0, 1, 10 }) ^
	boost::unit_test::data::make({ false, true, true }),
	N, is_valid)
{
	s << "{ \"type\" : \"FixedStoppingStrategy\",";
	s << "\"size\" : " << N << " }";

	ptree pt;
	read_json(s, pt);

	// try both functions (the second should just delegate to the
	// first).

	BOOST_TEST(try_load_fixed_stopping_strategy(*p_iter_mgr,
		pt) == is_valid);

	BOOST_TEST(try_load_stopping_strategies(*p_iter_mgr,
		pt) == is_valid);
}


BOOST_DATA_TEST_CASE(test_basic_stopping_strategy,
	(boost::unit_test::data::make({ 0, 1, 10 }) ^
	boost::unit_test::data::make({ false, true, true }))
	*
	(boost::unit_test::data::make({ -1.0f, 0.0f, 1.0f }) ^
	boost::unit_test::data::make({ false, false, true }))
	*
	(boost::unit_test::data::make({ -1.0f, 0.0f, 0.5f, 1.0f, 1.2f }) ^
	boost::unit_test::data::make({ false, false, true, false, false }))
	, N, N_is_valid, lambda, lambda_is_valid, alpha, alpha_is_valid)
{
	s << "{ \"type\" : \"BasicStoppingStrategy\",";
	s << "\"initial-size\" : " << N << ", ";
	s << "\"lambda\" : " << lambda << ", ";
	s << "\"alpha\" : " << alpha;
	s << " }";

	ptree pt;
	read_json(s, pt);

	// overall is valid iff all parameters are valid

	const bool is_valid = N_is_valid && lambda_is_valid
		&& alpha_is_valid;

	// try both functions (the second should just delegate to the
	// first).

	BOOST_TEST(try_load_basic_stopping_strategy(*p_iter_mgr,
		pt) == is_valid);

	BOOST_TEST(try_load_stopping_strategies(*p_iter_mgr,
		pt) == is_valid);
}


BOOST_AUTO_TEST_SUITE_END();


