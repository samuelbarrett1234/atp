/*

SyntaxTreeFoldTests.cpp

This file tests the following function:
- atp::logic::equational::fold_syntax_tree

*/


#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxTreeFold.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"


using atp::logic::equational::fold_syntax_tree;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::parse_statements;


struct SyntaxTreeFoldTestsFixture
{
	SyntaxTreeFoldTestsFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
	}

	std::stringstream s;
	KnowledgeKernel ker;
};


std::string stmts[] =
{
	"*(x0, x1) = e", "i(i(x0)) = x0",
	"*(x0, *(x1, x2)) = *(*(x0, x1), x2)",
	"*(e, e) = e"
};
size_t num_free_vars_in_stmts[] =
{
	2, 1, 3, 0
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SyntaxTreeFoldTests,
	SyntaxTreeFoldTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests"));


// test the fold function by seeing if we can use fold to build a
// to-string function for syntax trees, thus acting as an inverse
// to the parser!
// We may assume the parser is correct, because this test suite
// builds upon the parser
BOOST_DATA_TEST_CASE(test_to_str_is_inverse_to_parser,
	boost::unit_test::data::make(stmts) ^
	boost::unit_test::data::make(num_free_vars_in_stmts),
	stmt, num_free_vars)
{
	s << stmt;
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ker);

	// should have no problems here
	BOOST_REQUIRE(syntax_tree != nullptr);

	// warning: of course, we cannot guarantee the way in which the
	// free variable IDs get assigned, so we need to check that
	// some permutation of the variables ties up with the result
	// given here:

	BOOST_TEST(exists_free_var_assignment(ker,
		syntax_tree, stmt, num_free_vars));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


