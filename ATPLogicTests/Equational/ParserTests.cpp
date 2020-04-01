/*

ParserTests.cpp

This file will test two functions:
- atp::logic::equational::parse_statements
- atp::logic::equational::parse_definitions

*/


#include <sstream>
#include <boost/random/uniform_int_distribution.hpp>
#include <ATPLogic.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"


using atp::logic::equational::parse_statements;
using atp::logic::equational::parse_definitions;
using atp::logic::equational::ParseNodeType;
using atp::logic::equational::EqParseNode;
using atp::logic::equational::IdentifierParseNode;


struct ParserTestFixture
{
	std::stringstream s;
};


BOOST_FIXTURE_TEST_SUITE(ParserTests, ParserTestFixture)


std::string ill_formed_statements[] =
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


std::string ill_formed_definitions[] =
{
	"f\ng 0",
	"f -1",
	"1 f",
	"1",
	"f"
};


std::string symbol_names[] =
{
	"a", "b", "c", "f", "g", "h", "x", "y", "z",
	"*", "+", "-", ".", "/", "%"
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
	s << "f(x, y) = z # this is a comment! g(x, y) = z \n";
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
	s << "f(x, y)";

	auto result = parse_statements(s);

	// check it parsed exactly one statement
	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 1);

	auto stmt = result.get().front();

	// check that it got the root statement correct
	BOOST_REQUIRE(stmt->get_type() ==
		ParseNodeType::IDENTIFIER);

	auto p_func = dynamic_cast<IdentifierParseNode*>(
		stmt.get());

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


/////////////////////////////////////////////////////////////////////


// generate a random list of (string, int) pairs
BOOST_DATA_TEST_CASE(parse_definitions_returns_correct_mapping,
	boost::unit_test::data::random(boost::unit_test::data::distribution
		= boost::random::uniform_int_distribution<size_t>(0, 10))
	^ boost::unit_test::data::make(symbol_names), arity, name)
{
	s << name << ' ' << arity;

	auto result = parse_definitions(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().size() == 1);
	BOOST_TEST(result.get().front().first == name);
	BOOST_TEST(result.get().front().second == arity);
}


BOOST_AUTO_TEST_CASE(parse_definitions_works_with_none)
{
	// leave s empty

	auto result = parse_definitions(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_TEST(result.get().empty());
}


BOOST_DATA_TEST_CASE(parse_definitions_returns_none_when_incorrect,
	boost::unit_test::data::make(ill_formed_definitions), def)
{
	s << def;

	auto result = parse_definitions(s);

	BOOST_TEST(!result.has_value());
}


BOOST_AUTO_TEST_SUITE_END();


