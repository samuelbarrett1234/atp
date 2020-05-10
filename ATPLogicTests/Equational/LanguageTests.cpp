/**

\file

\author Samuel Barrett

\brief This file tests the equational::Language class.

*/


#include <sstream>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/ModelContext.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include "../Test.h"
#include "DefinitionFileExamples.h"


using atp::logic::equational::Language;
using atp::logic::equational::ModelContext;
using atp::logic::StmtFormat;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;


struct LanguageTestsFixture
{
	std::stringstream s;
	Language lang;

	std::stringstream ctx_file;

	LanguageTestsFixture()
	{
		s << std::noskipws;

		ctx_file << group_theory_definition_file;
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(LanguageTests,
	LanguageTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ModelContextTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseStatementsTests"));


BOOST_AUTO_TEST_CASE(check_no_ker_when_context_invalid)
{
	// an invalid axiom is not picked up by context file
	// loading, but should be picked up when we try to
	// create a knowledge kernel from that context file

	s << "{ \"axioms\" : [ \"x = \" ] }";

	auto p_ctx = lang.try_create_context(s);

	BOOST_REQUIRE(p_ctx != nullptr);

	auto p_ker = lang.try_create_kernel(*p_ctx);

	BOOST_TEST(p_ker == nullptr);
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
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	s << stmts;

	auto _lang_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
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
			ctx);

		BOOST_REQUIRE(stree != nullptr);

		// get statement
		auto stmt = Statement(ctx, stree);

		// test equivalent to the one produced by the language
		BOOST_TEST(stmt.equivalent(lang_stmts->my_at(i)));

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
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	s << stmt;
	auto lang_results = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

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
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	s << stmt;
	auto stmt_arr = lang.deserialise_stmts(s, StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(stmt_arr->size() == 1);

	// clear this
	s = std::stringstream();
	s << std::noskipws;

	lang.serialise_stmts(s, stmt_arr, StmtFormat::TEXT);

	BOOST_TEST(stmt_arr->at(0).to_str() == s.str());
}


BOOST_DATA_TEST_CASE(test_binary_serialisation,
	boost::unit_test::data::make({
		" x = i(x) \n *(x, y) = y",
		" *(*(x, y), z) = *(x, *(y, z))",
		" *(x, i(x)) = e \n e = e \n x = x",
		""  /* empty string! */,
		"*(x, y) = i(y) # a comment \n i(x) = i(i(x))" }),
	stmts_str)
{
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	s << stmts_str;

	auto stmts_by_text = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);

	std::stringstream mem_stream;

	lang.serialise_stmts(mem_stream, stmts_by_text,
		StmtFormat::BINARY);

	auto stmts_by_binary = lang.deserialise_stmts(mem_stream,
		StmtFormat::BINARY, ctx);

	BOOST_TEST(stmts_by_text->size() == stmts_by_binary->size());

	auto p1 = dynamic_cast<StatementArray*>(stmts_by_text.get());
	auto p2 = dynamic_cast<StatementArray*>(stmts_by_binary.get());

	for (size_t i = 0; i < p1->size(); ++i)
	{
		BOOST_TEST(p1->my_at(i).identical(p2->my_at(i)));
	}
}


BOOST_AUTO_TEST_CASE(test_normalise_reduces_free_ids)
{
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	s << "*(x2, *(x3, x7)) = *(*(x2, x7), e)";

	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);

	auto normalised = lang.normalise(stmts);

	BOOST_REQUIRE(normalised->size() == 1);

	// there are many valid answers to this test
	// (just the six permutations of the three variables)
	std::string possible_answers[] =
	{
		"*(x0, *(x1, x2)) = *(*(x0, x2), e)",
		"*(x1, *(x0, x2)) = *(*(x1, x2), e)",
		"*(x2, *(x1, x0)) = *(*(x2, x0), e)",
		"*(x1, *(x2, x0)) = *(*(x1, x0), e)",
		"*(x2, *(x0, x1)) = *(*(x2, x1), e)",
		"*(x0, *(x2, x1)) = *(*(x0, x1), e)"
	};

	std::string result = normalised->at(0).to_str();

	BOOST_TEST((std::find(possible_answers,
		possible_answers + 6, result) != possible_answers + 6));
}


BOOST_AUTO_TEST_CASE(test_normalise_reduces_about_equals_sign)
{
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	// do this in two seperate runs, because `normalise` also reduces
	// by equivalence within the call

	s << "e = i(e) \n i(e) = e";

	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);

	auto normalised1 = lang.normalise(stmts->slice(0, 1));
	auto normalised2 = lang.normalise(stmts->slice(1, 2));

	BOOST_REQUIRE(normalised1->size() == 1);
	BOOST_REQUIRE(normalised2->size() == 1);

	// check that it reduced it around the equals sign in a
	// consistent way:

	BOOST_TEST(normalised1->at(0).to_str()
		== normalised2->at(0).to_str());
}


BOOST_AUTO_TEST_CASE(test_normalise_reduces_by_equivalence)
{
	auto p_ctx = lang.try_create_context(ctx_file);
	BOOST_REQUIRE(p_ctx != nullptr);
	const auto& ctx = dynamic_cast<const ModelContext&>(*p_ctx);

	// the first two are equivalent but the third is not
	s << "e = *(x0, i(x0)) \n *(x1, i(x1)) = e \n *(i(x0), x0) = e";

	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);

	auto normalised = lang.normalise(stmts);

	BOOST_REQUIRE(normalised->size() == 2);

	BOOST_TEST((normalised->at(0).to_str() == "e = *(x0, i(x0))"
		|| normalised->at(0).to_str() == "*(x0, i(x0)) = e"));

	BOOST_TEST((normalised->at(1).to_str() == "e = *(i(x0), x0)"
		|| normalised->at(1).to_str() == "*(i(x0), x0) = e"));
}


BOOST_AUTO_TEST_SUITE_END();  // LanguageTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


