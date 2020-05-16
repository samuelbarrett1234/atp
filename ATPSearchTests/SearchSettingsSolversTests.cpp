/**
\file

\author Samuel Barrett

\brief Test suite for solver settings parsing
*/


#include <Internal/SearchSettingsSolvers.h>
#include <Internal/IteratorManager.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "Test.h"
#include "LogicSetupFixture.h"


using boost::property_tree::ptree;
using boost::property_tree::read_json;
using atp::search::IteratorManager;
using atp::search::try_create_solver;
using atp::search::try_get_flags;
using atp::search::try_create_IDS;
using atp::search::SolverCreator;
using atp::logic::IterSettings;
namespace iter_settings = atp::logic::iter_settings;


struct SearchSettingsSolversTestsFixture :
	public LogicSetupFixture
{
	std::unique_ptr<IteratorManager> p_iter_mgr =
		std::make_unique<IteratorManager>(p_ker);
};


BOOST_AUTO_TEST_SUITE(SearchSettingsTests);
BOOST_FIXTURE_TEST_SUITE(SearchSettingsSolversTests,
	SearchSettingsSolversTestsFixture,
	* boost::unit_test_framework::depends_on(
	"SolverTests"));


BOOST_AUTO_TEST_CASE(test_bad_solver_name)
{
	s << "{ \"type\" : \"bad-solver-name\" }";

	ptree pt;
	read_json(s, pt);

	SolverCreator creator;

	BOOST_TEST(!try_create_solver(pt,
		creator));

	BOOST_TEST((!(bool)creator));
}


BOOST_DATA_TEST_CASE(test_iter_settings,
	boost::unit_test::data::make({ false, true }) *
	boost::unit_test::data::make({ false, true }),
	no_repeats, randomised)
{
	s << "{ \"no-repeats\" : " << (no_repeats ? "true" : "false")
		<< ", \"randomised\" : " << (randomised ? "true" : "false")
		<< " }";

	ptree pt;
	read_json(s, pt);

	auto result_flags = try_get_flags(pt);

	IterSettings true_flags = iter_settings::DEFAULT;

	if (no_repeats)
		true_flags |= iter_settings::NO_REPEATS;
	else
		true_flags &= ~iter_settings::NO_REPEATS;

	if (randomised)
		true_flags |= iter_settings::RANDOMISED;
	else
		true_flags &= ~iter_settings::RANDOMISED;

	BOOST_TEST(result_flags == true_flags);
}


BOOST_AUTO_TEST_CASE(test_default_iter_settings)
{
	s << "{}";

	ptree pt;
	read_json(s, pt);

	auto result_flags = try_get_flags(pt);

	BOOST_TEST(result_flags == iter_settings::DEFAULT);
}


// tries lots of invalid parameters to make sure it handles them
// gracefully
BOOST_DATA_TEST_CASE(test_create_ids,
	(boost::unit_test::data::make({
		0, 1, 5, 5, 5, 5, 5 }) ^ boost::unit_test::data::make({
		0, 1, 5, 0, 1, 4, 4 }) ^ boost::unit_test::data::make({
		9, 9, 9, 9, 9, 9, 0 }) ^ boost::unit_test::data::make({
		2, 2, 2, 2, 2, 2, 2 }) ^ boost::unit_test::data::make({
		false, false, false, false, false, true, false }))
	* boost::unit_test::data::make({ false, true })
	* boost::unit_test::data::make({ false, true }),
	max_depth, starting_depth, width_limit, width_limit_start, is_valid, no_repeats, randomised)
{
	s << "{ \"type\" : \"IterativeDeepeningSolver\",";
	s << "\"max-depth\" : " << max_depth << ", ";
	s << "\"starting-depth\" : " << starting_depth << ", ";
	s << "\"width-limit\" : " << width_limit << ", ";
	s << "\"width-limit-start-depth\" : " << width_limit_start << ", ";
	s << "\"no-repeats\" : " << (no_repeats ? "true" : "false") << ", ";
	s << "\"randomised\" : " << (randomised ? "true" : "false");
	s << "}";

	ptree pt;
	read_json(s, pt);

	SolverCreator creator;

	BOOST_TEST(try_create_IDS(pt,
		creator, iter_settings::DEFAULT) == is_valid);

	BOOST_TEST(((bool)creator) == is_valid);

	if ((bool)creator)
	{
		auto p_result = creator(p_ker, std::move(p_iter_mgr));

		BOOST_TEST((p_result != nullptr) == is_valid);
	}
}


// test that the generic version can create an IDS
BOOST_AUTO_TEST_CASE(test_create_solver_can_make_ids,
	* boost::unit_test_framework::depends_on(
		"SearchSettingsTests/SearchSettingsSolversTests/test_create_ids"))
{
	// use defaults for all other parameters
	s << "{ \"type\" : \"IterativeDeepeningSolver\" }";

	ptree pt;
	read_json(s, pt);

	SolverCreator creator;

	BOOST_TEST(try_create_solver(pt,
		creator));

	BOOST_REQUIRE((bool)creator);

	auto p_result = creator(p_ker, std::move(p_iter_mgr));

	BOOST_TEST(p_result != nullptr);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


