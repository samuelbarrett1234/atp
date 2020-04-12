/**

\file

\author Samuel Barrett

\brief This file tests `equational::ModelContext`

*/


#include <sstream>
#include <Internal/Equational/ModelContext.h>
#include <Internal/Equational/Language.h>
#include "../Test.h"


using atp::logic::equational::ModelContext;
using atp::logic::equational::Language;


struct ModelContextTestsFixture
{
	std::stringstream s;
	Language lang;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ModelContextTests,
	ModelContextTestsFixture);


BOOST_AUTO_TEST_CASE(definition_name_test)
{
	s << "{ \"definitions\" : [ { \"name\" : \"f\", \"arity\" : 2 } ] }";

	auto p_ctx = ModelContext::try_construct(lang,
		s);

	BOOST_REQUIRE(p_ctx != nullptr);

	BOOST_TEST(p_ctx->is_defined("f"));
}


BOOST_AUTO_TEST_CASE(arity_test)
{
	s << "{ \"definitions\" : [ { \"name\" : \"f\", \"arity\" : 2 } ] }";

	auto p_ctx = ModelContext::try_construct(lang,
		s);

	BOOST_REQUIRE(p_ctx != nullptr);

	BOOST_TEST(p_ctx->symbol_arity("f") == 2);
	BOOST_TEST(p_ctx->symbol_arity(
		p_ctx->symbol_id("f")) == 2);
}


BOOST_AUTO_TEST_CASE(empty_test)
{
	// empty files count as an error

	s << "";

	auto p_ctx = ModelContext::try_construct(lang, s);

	BOOST_REQUIRE(p_ctx == nullptr);
}


BOOST_AUTO_TEST_CASE(syntax_error_test)
{
	s << "{";

	auto p_ctx = ModelContext::try_construct(lang, s);

	BOOST_TEST(p_ctx == nullptr);
}


BOOST_AUTO_TEST_CASE(axiom_test)
{
	s << "{ \"axioms\" : [ \"x=x\" ] }";

	auto p_ctx = ModelContext::try_construct(lang, s);

	BOOST_REQUIRE(p_ctx != nullptr);

	BOOST_TEST(p_ctx->num_axioms() == 1);

	BOOST_TEST(p_ctx->axiom_at(0) == "x=x");
}


BOOST_AUTO_TEST_CASE(definition_name_trim_test)
{
	s << "{ \"definitions\" : [ { \"name\" : \"   e  \", \"arity\" : 0 } ] }";

	auto p_ctx = ModelContext::try_construct(lang, s);

	BOOST_REQUIRE(p_ctx != nullptr);

	BOOST_TEST(p_ctx->is_defined("e"));
	BOOST_TEST(!p_ctx->is_defined("   e  \t\n "));
}


BOOST_AUTO_TEST_CASE(redefinition_failure_test)
{
	s << "{ \"definitions\" : [ { \"name\" : \"f\", \"arity\" : 2 }, "
		"{ \"name\" : \"f\", \"arity\" : 2 } ] }";

	auto p_ctx = ModelContext::try_construct(lang, s);

	BOOST_TEST(p_ctx == nullptr);
}


BOOST_AUTO_TEST_SUITE_END();  // ModelContextTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


