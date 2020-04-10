/*

LanguageTests.cpp

This file tests the equational::Language class.

*/


#include <sstream>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/Semantics.h>
#include "../Test.h"


using atp::logic::equational::Language;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::StmtFormat;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
namespace semantics = atp::logic::equational::semantics;


struct LanguageTestsFixture
{
	std::stringstream s;
	KnowledgeKernel ker;
	Language lang;

	LanguageTestsFixture()
	{
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
		s << std::noskipws;
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(LanguageTests,
	LanguageTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseDefinitionsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SemanticsTests"));


BOOST_AUTO_TEST_CASE(check_integrity_of_empty_kernel)
{
	KnowledgeKernel definitely_empty;
	auto p_ker = lang.create_empty_kernel();

	BOOST_TEST(definitely_empty.get_integrity_code() ==
		p_ker->get_integrity_code());
}


BOOST_AUTO_TEST_CASE(check_definition_loading)
{
	KnowledgeKernel empty_ker;

	s << "e 0 \n i 1 \n * 2";
	BOOST_TEST(lang.load_kernel_definitions(empty_ker, s));
	
	// check the definitions worked:

	BOOST_TEST(empty_ker.is_defined("e"));
	BOOST_TEST(empty_ker.is_defined("i"));
	BOOST_TEST(empty_ker.is_defined("*"));

	// note that the `ker` from the fixture already comes
	// with the group theory definitions
	BOOST_TEST(empty_ker.get_integrity_code() ==
		ker.get_integrity_code());
}


BOOST_AUTO_TEST_CASE(check_definition_loading_when_incorrect)
{
	KnowledgeKernel empty_ker;

	s << "e \n 1 i";  // bogus definition file

	BOOST_TEST(!lang.load_kernel_definitions(empty_ker, s));
}


BOOST_AUTO_TEST_CASE(check_definition_no_partial_load)
{
	// check that, if any definition fails, the kernel remains
	// empty
	KnowledgeKernel empty_ker1, empty_ker2;

	s << "e 0 \n 1 i";  // first definition correct, second not

	BOOST_TEST(!lang.load_kernel_definitions(empty_ker1, s));

	BOOST_TEST(!empty_ker1.is_defined("e"));
	BOOST_TEST(empty_ker1.get_integrity_code() ==
		empty_ker2.get_integrity_code());
}


BOOST_DATA_TEST_CASE(test_text_deserialisation_in_correct_cases,
	boost::unit_test::data::make({
		" x = i(x) \n *(x, y) = y",
		" *(*(x, y), z) = *(x, *(y, z))",
		" *(x, i(x)) = e \n e = e \n x = x",
		""  /* empty string! */,
		"*(x, y) = i(y) # a comment \n i(x) = i(i(x))"}),
	stmts)
{
	s << stmts;

	auto _lang_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ker);
	auto lang_stmts = dynamic_cast<StatementArray*>(
		_lang_stmts.get());

	BOOST_REQUIRE(lang_stmts != nullptr);

	// reset stream
	s = std::stringstream();
	s << std::noskipws;
	s << stmts;

	// now do the deserialisation ourselves manually

	auto parse_results = parse_statements(s);

	BOOST_REQUIRE(parse_results.has_value());
	BOOST_REQUIRE(parse_results.get().size() ==
		lang_stmts->size());

	size_t i = 0;
	for (auto ptree : parse_results.get())
	{
		// get syntax tree
		auto stree = ptree_to_stree(ptree,
			ker);

		BOOST_REQUIRE(stree != nullptr);

		// get statement
		auto stmt = Statement(ker, stree);

		// test equivalent to the one produced by the language
		BOOST_TEST(semantics::equivalent(stmt,
			lang_stmts->my_at(i)));

		i++;
	}
}


BOOST_DATA_TEST_CASE(test_text_deserialisation_in_incorrect_cases,
	boost::unit_test::data::make({
		"x = x \n f(x)",
		"f(x) = y  # free function f",
		"*(x, y) = y  # valid \n i(*) = y"}),
	stmt)
{
	s << stmt;
	auto lang_results = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	// if one statement is wrong, no array is returned at all, even
	// if some statements are correct!
	BOOST_TEST(lang_results == nullptr);
}



// for now, only test the special case where there is at most
// one free variable, to avoid issues with having several possible
// equivalent (and correct) ways of assigning names to the free
// variables.
BOOST_DATA_TEST_CASE(test_serialisation_in_one_free_variable,
	boost::unit_test::data::make({ "x0 = x0", "i(x0) = x0",
		"*(x0, i(x0)) = e", "e = e", "*(e, e) = e" }),
	stmt)
{
	s << stmt;
	auto stmt_arr = lang.deserialise_stmts(s, StmtFormat::TEXT, ker);

	BOOST_REQUIRE(stmt_arr->size() == 1);

	// clear this
	s = std::stringstream();
	s << std::noskipws;

	lang.serialise_stmts(s, stmt_arr, StmtFormat::TEXT);

	BOOST_TEST(stmt_arr->at(0).to_str() == s.str());
}


BOOST_AUTO_TEST_SUITE_END();  // LanguageTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


