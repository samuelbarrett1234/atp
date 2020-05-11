/**
\file

\author Samuel Barrett

\brief Test suite for solver settings parsing
*/


#include <Internal/SearchSettingsHeuristics.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Test.h"
#include "LogicSetupFixture.h"


using boost::property_tree::ptree;
using boost::property_tree::read_json;
using atp::search::HeuristicCreator;
using atp::search::HeuristicPtr;
using atp::search::try_create_heuristic;
using atp::search::try_create_edit_distance_heuristic;


BOOST_AUTO_TEST_SUITE(SearchSettingsTests);
BOOST_FIXTURE_TEST_SUITE(SearchSettingsHeuristicsTests,
	LogicSetupFixture);


BOOST_AUTO_TEST_CASE(test_bad_heuristic_name)
{
	s << "{ \"type\" : \"bad-heuristic-name\" }";

	ptree pt;
	read_json(s, pt);

	HeuristicCreator creator;

	BOOST_TEST(!try_create_heuristic(p_ctx,
		creator, pt));

	BOOST_TEST((!(bool)creator));
}


// tries lots of invalid parameters to make sure it handles them
// gracefully
BOOST_DATA_TEST_CASE(test_create_edit_distance_heuristic,
	boost::unit_test::data::make({
		-1.0f, 0.0f, 1.0f }) *
	boost::unit_test::data::make({
		-1.0f, 0.0f, 1.0f }),
	p, symb_mismatch_cost)
{
	const bool correct = (p > 0.0f && symb_mismatch_cost > 0.0f);

	s << "{ \"type\" : \"EditDistanceHeuristic\",";
	s << "\"p\" : " << p << ", ";
	s << "\"symbol-mismatch-cost\" : " << symb_mismatch_cost;
	s << "}";

	ptree pt;
	read_json(s, pt);

	HeuristicCreator creator;

	BOOST_TEST(try_create_heuristic(p_ctx,
		creator, pt) == correct);

	BOOST_TEST(((bool)creator) == correct);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


