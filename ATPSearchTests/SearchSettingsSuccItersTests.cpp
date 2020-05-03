/*
\file

\author Samuel Barrett

\brief Test suite for stopping strategy parsing in search settings
	files.

*/


#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <Internal/SearchSettingsSuccIters.h>
#include <Internal/IteratorManager.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::search::try_load_succ_iter_settings;
using atp::search::try_load_fixed_stopping_strategy;
using atp::search::try_load_basic_stopping_strategy;
using atp::search::IteratorManager;
using atp::search::SuccIterCreator;
using boost::property_tree::ptree;
using boost::property_tree::read_json;


struct SearchSettingsSuccItersTestsFixture :
	public LogicSetupFixture
{
	std::unique_ptr<IteratorManager> p_iter_mgr =
		std::make_unique<IteratorManager>(p_ker);

	std::stringstream s;
};


BOOST_AUTO_TEST_SUITE(SearchSettingsTests);
BOOST_FIXTURE_TEST_SUITE(SearchSettingsSuccItersTests,
	SearchSettingsSuccItersTestsFixture,
	* boost::unit_test_framework::depends_on(
	"SuccessorIteratorTests"));


BOOST_AUTO_TEST_CASE(test_bad_strat_type)
{
	s << "{ \"type\" : \"bad-strat-type\" }";

	ptree pt;
	read_json(s, pt);

	SuccIterCreator creator;

	BOOST_TEST(!try_load_succ_iter_settings(
		creator, pt));

	BOOST_TEST(!(bool)creator);
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
	SuccIterCreator creator;

	BOOST_TEST(try_load_fixed_stopping_strategy(creator,
		pt) == is_valid);

	BOOST_TEST(try_load_succ_iter_settings(creator,
		pt) == is_valid);
}


BOOST_DATA_TEST_CASE(test_basic_stopping_strategy,
	(boost::unit_test::data::make({ 0, 1, 2, 10 }) ^
	boost::unit_test::data::make({ false, false, true, true }))
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

	SuccIterCreator creator;

	BOOST_TEST(try_load_basic_stopping_strategy(creator,
		pt) == is_valid);

	BOOST_TEST(try_load_succ_iter_settings(creator,
		pt) == is_valid);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


