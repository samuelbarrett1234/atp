/**

\file

\author Samuel Barrett

\brief This file tests atp::logic::equational::fold_syntax_tree 

*/


#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/SyntaxTreeFold.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"
#include "StandardFixture.h"


using atp::logic::equational::fold_syntax_tree;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::parse_statements;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SyntaxTreeFoldTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests"));


// test the fold function by seeing if we can use fold to build a
// to-string function for syntax trees, thus acting as an inverse
// to the parser!
// We may assume the parser is correct, because this test suite
// builds upon the parser


BOOST_AUTO_TEST_CASE(test_to_str_is_inverse_to_parser_1)
{
	s << "*(x0, x1) = e";
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ctx);

	// should have no problems here
	BOOST_REQUIRE(syntax_tree != nullptr);

	// warning: of course, we cannot guarantee the way in which the
	// free variable IDs get assigned, so we need to check that
	// some permutation of the variables ties up with the result
	// given here:

	auto to_str = syntax_tree_to_str(ctx, syntax_tree);

	BOOST_TEST((to_str == "*(x0, x1) = e" ||
		to_str == "*(x1, x0) = e"));
}


BOOST_AUTO_TEST_CASE(test_to_str_is_inverse_to_parser_2)
{
	s << "i(i(x0)) = x0";
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ctx);

	// should have no problems here
	BOOST_REQUIRE(syntax_tree != nullptr);

	// warning: of course, we cannot guarantee the way in which the
	// free variable IDs get assigned, so we need to check that
	// some permutation of the variables ties up with the result
	// given here:

	auto to_str = syntax_tree_to_str(ctx, syntax_tree);

	BOOST_TEST(to_str == "i(i(x0)) = x0");
}


BOOST_AUTO_TEST_CASE(test_to_str_is_inverse_to_parser_3)
{
	s << "*(e, e) = e";
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ctx);

	// should have no problems here
	BOOST_REQUIRE(syntax_tree != nullptr);

	// warning: of course, we cannot guarantee the way in which the
	// free variable IDs get assigned, so we need to check that
	// some permutation of the variables ties up with the result
	// given here:

	auto to_str = syntax_tree_to_str(ctx, syntax_tree);

	BOOST_TEST(to_str == "*(e, e) = e");
}


BOOST_AUTO_TEST_CASE(test_to_str_is_inverse_to_parser_4)
{
	// note: this statement catches an off-by-one
	// bug in the fold (specifically, when the function
	// constructor pops its children off the result
	// stack)
	s << "*(x, y) = i(*(i(x), i(y)))";
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ctx);

	// should have no problems here
	BOOST_REQUIRE(syntax_tree != nullptr);

	// warning: of course, we cannot guarantee the way in which the
	// free variable IDs get assigned, so we need to check that
	// some permutation of the variables ties up with the result
	// given here:

	auto to_str = syntax_tree_to_str(ctx, syntax_tree);

	BOOST_TEST((to_str == "*(x0, x1) = i(*(i(x0), i(x1)))" ||
		to_str == "*(x1, x0) = i(*(i(x1), i(x0)))"));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


