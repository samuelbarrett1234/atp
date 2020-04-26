/**

\file

\author Samuel Barrett

\brief This suite tests the Search Settings parsing.

*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPSearch.h>
#include "Test.h"


using atp::search::SearchSettings;
using atp::search::load_search_settings;
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
	* boost::unit_test_framework::depends_on(
	"SearchSettingsTests/SearchSettingsSolversTests")
	* boost::unit_test_framework::depends_on(
		"SearchSettingsTests/SearchSettingsSuccItersTests"));


BOOST_AUTO_TEST_CASE(test_no_solver)
{
	s << "{";
	s << "\"name\" : \"test-name\",";
	s << "\"desc\" : \"test-desc\",";
	s << "\"max-steps\" : 1,";
	s << "\"step-size\" : 2";
	s << "}";

	SearchSettings settings;
	BOOST_TEST(load_search_settings(p_ker, s,
		&settings));
	BOOST_TEST(!((bool)settings.create_solver));
}


BOOST_AUTO_TEST_CASE(test_get_step_settings)
{
	s << "{";
	s << "\"name\" : \"test-name\",";
	s << "\"desc\" : \"test-desc\",";
	s << "\"max-steps\" : 1,";
	s << "\"step-size\" : 2";
	s << "}";

	SearchSettings settings;
	BOOST_TEST(load_search_settings(p_ker, s,
		&settings));
	BOOST_TEST(settings.name == "test-name");
	BOOST_TEST(settings.desc == "test-desc");
	BOOST_TEST(settings.max_steps == 1);
	BOOST_TEST(settings.step_size == 2);
}


BOOST_AUTO_TEST_CASE(test_bad_solver_name)
{
	s << "{ \"solver\" : {";
	s << "\"type\" : \"bad-solver-name\"";
	s << "} }";

	SearchSettings settings;
	BOOST_TEST(!load_search_settings(p_ker, s,
		&settings));
}


BOOST_AUTO_TEST_CASE(test_error_when_stop_strat_but_no_heuristic)
{
	// don't provide a heuristic!
	s << "{ \"stopping-strategy\" : {";
	s << "\"type\" : \"FixedStoppingStrategy\", \"size\" : 5";
	s << "}, \"solver\" : { \"type\" : \"IterativeDeepeningSolver\"";
	s << " }";

	SearchSettings settings;
	BOOST_TEST(!load_search_settings(p_ker, s,
		&settings));
}


BOOST_AUTO_TEST_CASE(test_bad_json_syntax)
{
	s << "{ bad : 7 }";

	SearchSettings settings;
	BOOST_TEST(!load_search_settings(p_ker, s,
		&settings));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


