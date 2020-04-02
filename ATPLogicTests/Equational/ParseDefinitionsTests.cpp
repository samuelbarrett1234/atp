/*

ParseDefinitionsTests.cpp

This file will test the function:
- atp::logic::equational::parse_definitions

*/


#include <sstream>
#include <boost/random/uniform_int_distribution.hpp>
#include <ATPLogic.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"


using atp::logic::equational::parse_definitions;


struct ParseDefinitionsTestFixture
{
	ParseDefinitionsTestFixture()
	{
		s << std::noskipws;
	}
	std::stringstream s;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ParseDefinitionsTests, ParseDefinitionsTestFixture)


static std::string ill_formed_definitions[] =
{
	"f\ng 0",
	"f -1",
	"1 f",
	"1",
	"f"
};


static std::string symbol_names[] =
{
	"a", "b", "c", "f", "g", "h", "x", "y", "z",
	"*", "+", "-", ".", "/", "%"
};


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


BOOST_AUTO_TEST_CASE(test_no_partial_load)
{
	// if one line is incorrect then it counts as a
	// failed parse

	s << "e 0 \n 1 i";

	auto result = parse_definitions(s);

	BOOST_TEST(!result.has_value());
}


BOOST_DATA_TEST_CASE(parse_definitions_returns_none_when_incorrect,
	boost::unit_test::data::make(ill_formed_definitions), def)
{
	s << def;

	auto result = parse_definitions(s);

	BOOST_TEST(!result.has_value());
}


BOOST_AUTO_TEST_SUITE_END();  // ParseDefinitionsTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


