/*

ParseStatementsTests.cpp

This file will test the function:
- atp::logic::equational::parse_statements

*/


#include <sstream>
#include <ATPLogic.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"


using atp::logic::equational::parse_statements;
using atp::logic::equational::ParseNodeType;
using atp::logic::equational::EqParseNode;
using atp::logic::equational::IdentifierParseNode;


struct ParseStatementsTestFixture
{
	ParseStatementsTestFixture()
	{
		s << std::noskipws;
	}
	std::stringstream s;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ParseStatementsTests, ParseStatementsTestFixture)


static std::string ill_formed_statements[] =
{
	"f(x,",
	"f(",
	"f()",
	"f(,x)",
	"x=",
	"=x",
	"f(x=y,z)",
	"f(x=g(x))",
	"f x,y"
};


BOOST_AUTO_TEST_CASE(parse_statements_returns_empty_results_but_some)
{
	// leave s empty
	auto result = parse_statements(s);

	// we should have a result, but that result should be empty:
	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().empty());
}


BOOST_AUTO_TEST_CASE(parse_statements_returns_one_result)
{
	s << "f(x, y) = z";

	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 1);
}


BOOST_AUTO_TEST_CASE(parse_statements_handles_no_spaces)
{
	s << "x=x\ny=y";

	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 2);
}


BOOST_AUTO_TEST_CASE(parse_statements_handles_trailing_whitespace)
{
	s << "x = x\n\n\n\n\n\n    \t\t";

	auto result = parse_statements(s);

	BOOST_TEST(result.has_value());
}


BOOST_AUTO_TEST_CASE(parse_statements_handles_words_and_numbers)
{
	s << "very_long_func_name(x, very_b1g_var1abl3_nam3) = 3";

	auto result = parse_statements(s);

	BOOST_TEST(result.has_value());
}


BOOST_AUTO_TEST_CASE(parse_statements_returns_three_results)
{
	s << "f(x, y) = z\ng(z)=f(x, y)\nh(x, y, z, w) = f(x, y)";

	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 3);
}


BOOST_AUTO_TEST_CASE(parse_statements_handles_special_chars)
{
	// just check we don't get a parse error for these
	// special, yet common, function names:
	s << "*(a, b) = c \n +(a, b) = c \n .(a, b) = c \n";
	s << "%(a, b) = c \n f-(a, b) = c \n g*(x) = y \n";
	s << "/(a, b) = *(a, /(1, b))";

	auto result = parse_statements(s);

	BOOST_TEST(result.has_value());
}


BOOST_AUTO_TEST_CASE(parse_statements_ignores_comment_at_eol)
{
	// note that g(x, y) = z should be ignored here:
	s << "f(x, y) = z # this is a comment! g(x, y) = z" << std::endl;
	s << "h(x, y) = z";

	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 2);
}


BOOST_DATA_TEST_CASE(parse_statements_returns_none_when_incorrect,
	boost::unit_test::data::make(ill_formed_statements), stmt)
{
	s << stmt;

	auto result = parse_statements(s);

	BOOST_TEST(!result.has_value());
}


BOOST_AUTO_TEST_CASE(parse_statements_correctly_interprets_equality)
{
	s << "x = y";

	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 1);

	auto stmt = result.get().front();

	BOOST_REQUIRE(stmt->get_type() == ParseNodeType::EQ);

	auto p_eq = dynamic_cast<EqParseNode*>(
		stmt.get());

	BOOST_REQUIRE(p_eq != nullptr);

	BOOST_REQUIRE(p_eq->left()->get_type() ==
		ParseNodeType::IDENTIFIER);
	BOOST_REQUIRE(p_eq->right()->get_type() ==
		ParseNodeType::IDENTIFIER);

	auto p_left = dynamic_cast<IdentifierParseNode*>(
		p_eq->left().get());
	auto p_right = dynamic_cast<IdentifierParseNode*>(
		p_eq->right().get());

	BOOST_TEST(p_left->get_name() == "x");
	BOOST_TEST(p_right->get_name() == "y");
}


BOOST_AUTO_TEST_CASE(parse_statements_correctly_interprets_functions)
{
	s << "f(x, y) = z";

	auto result = parse_statements(s);

	// check it parsed exactly one statement
	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 1);

	auto stmt = result.get().front();

	BOOST_REQUIRE(stmt->get_type() == ParseNodeType::EQ);

	auto p_eq = dynamic_cast<EqParseNode*>(
		stmt.get());

	BOOST_REQUIRE(p_eq != nullptr);

	// check that it got the root statement correct
	BOOST_REQUIRE(p_eq->left()->get_type() ==
		ParseNodeType::IDENTIFIER);

	auto p_func = dynamic_cast<IdentifierParseNode*>(
		p_eq->left().get());

	BOOST_REQUIRE(p_func != nullptr);

	// check that it got the details of the function correct
	// (i.e. number of arguments, and its name):
	BOOST_TEST(std::distance(p_func->begin(),
		p_func->end()) == 2);

	BOOST_TEST(p_func->get_name() == "f");

	// now examine the children:
	auto iter = p_func->begin();

	// ensure there is at least one child:
	BOOST_REQUIRE(iter != p_func->end());

	// check left (first) child was correctly parsed:
	BOOST_REQUIRE((*iter)->get_type() ==
		ParseNodeType::IDENTIFIER);

	auto p_child = dynamic_cast<IdentifierParseNode*>(
		iter->get());

	BOOST_REQUIRE(p_child != nullptr);

	BOOST_TEST(p_child->get_name() == "x");

	// go to next child
	std::advance(iter, 1);

	// ensure we've not reached the end:
	BOOST_REQUIRE(iter != p_func->end());

	// check right (second) child was correctly parsed
	BOOST_REQUIRE((*iter)->get_type() ==
		ParseNodeType::IDENTIFIER);

	p_child = dynamic_cast<IdentifierParseNode*>(
		iter->get());

	BOOST_REQUIRE(p_child != nullptr);

	BOOST_TEST(p_child->get_name() == "y");
}


BOOST_AUTO_TEST_CASE(test_no_partial_load)
{
	// if one line is incorrect then it counts as a
	// failed parse

	s << "x = x \n x = i(";

	auto result = parse_statements(s);

	BOOST_TEST(!result.has_value());
}


BOOST_AUTO_TEST_CASE(test_lots_of_newlines_and_comments)
{
	s << "\n\n\n\n\n";
	s << "# this is a comment on a line by itself.\n\n";
	s << "x = y\n";
	s << "# this is a comment at the end";

	auto result = parse_statements(s);

	BOOST_TEST(result.has_value());
}


BOOST_AUTO_TEST_SUITE_END();  // ParseStatementTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests