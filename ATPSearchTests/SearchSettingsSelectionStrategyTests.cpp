/**

\file

\author Samuel Barrett

\brief This suite tests the Search Settings parsing.

*/


// disable BOOST_LOG_DYN_LINK redefinition
#pragma warning (disable:4005)
#include <Internal/SearchSettingsSelectionStrategies.h>
#pragma warning (default:4005)
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <ATPLogic.h>
#include "Test.h"


using boost::property_tree::ptree;
using boost::property_tree::read_json;
using atp::search::try_create_selection_strategy;
using atp::search::try_create_fixed_selection_strategy;
using atp::search::try_create_edit_dist_selection_strategy;
using atp::search::SelectionStrategyCreator;
using atp::logic::create_language;
using atp::logic::LangType;
using atp::logic::LanguagePtr;
using atp::logic::ModelContextPtr;
using atp::logic::KnowledgeKernelPtr;


struct SearchSettingsTestFixture
{
	std::stringstream s;
	LanguagePtr p_lang;
	ModelContextPtr p_ctx;
	KnowledgeKernelPtr p_ker;

	SearchSettingsTestFixture()
	{
		// empty context file
		std::stringstream ctx_in("{}");

		p_lang = create_language(
			LangType::EQUATIONAL_LOGIC);
		p_ctx = p_lang->try_create_context(ctx_in);
		p_ker = p_lang->try_create_kernel(*p_ctx);
	}
};


BOOST_AUTO_TEST_SUITE(SearchSettingsTests);
BOOST_FIXTURE_TEST_SUITE(CoreSearchSettingsTests,
	SearchSettingsTestFixture,
	*boost::unit_test_framework::depends_on(
		"SearchSettingsTests/SearchSettingsSolversTests")
	* boost::unit_test_framework::depends_on(
		"SearchSettingsTests/SearchSettingsSuccItersTests"));


BOOST_AUTO_TEST_CASE(test_bad_ss_name)
{
	s << "{ \"type\" : \"bad-ss-name\" }";

	ptree pt;
	read_json(s, pt);

	SelectionStrategyCreator creator;

	BOOST_TEST(!try_create_selection_strategy(pt,
		creator));

	BOOST_TEST((!(bool)creator));
}


BOOST_AUTO_TEST_CASE(test_bad_no_ss_name)
{
	s << "{ \"type\" : \"\" }";

	ptree pt;
	read_json(s, pt);

	SelectionStrategyCreator creator;

	BOOST_TEST(!try_create_selection_strategy(pt,
		creator));

	BOOST_TEST((!(bool)creator));
}


BOOST_AUTO_TEST_CASE(test_fixed)
{
	s << "{ \"type\" : \"FixedSelectionStrategy\" }";

	ptree pt;
	read_json(s, pt);

	SelectionStrategyCreator creator;

	BOOST_TEST(try_create_selection_strategy(pt,
		creator));

	BOOST_TEST((bool)creator);
}


BOOST_AUTO_TEST_CASE(test_edit_dist)
{
	s << "{ \"type\" : \"EditDistanceSelectionStrategy\" }";

	ptree pt;
	read_json(s, pt);

	SelectionStrategyCreator creator;

	BOOST_TEST(try_create_selection_strategy(pt,
		creator));

	BOOST_TEST((bool)creator);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


