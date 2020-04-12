/**

\file

\author Samuel Barrett

\brief This suite tests the function `atp::logic::equational::fold_parse_tree`

*/


#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/ParseTreeFold.h>
#include <Internal/Equational/Parser.h>
#include "../Test.h"


using atp::logic::equational::fold_parse_tree;
using atp::logic::equational::parse_statements;
using atp::logic::equational::EqParseNode;
using atp::logic::equational::IdentifierParseNode;


static std::string stmts[] =
{
	"f(x, y) = z",
	"*(x, i(x)) = e"
};


// helper functions for converting parse nodes to string:
static std::string eq_to_str(std::string lhs, std::string rhs);
static std::string identifier_to_str(std::string name,
	std::vector<std::string>::iterator arg_begin,
	std::vector<std::string>::iterator arg_end);


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_AUTO_TEST_SUITE(ParseTreeFoldTests,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseStatementsTests"));


// the idea for this test is to: get a load of statements in string
// form, convert them to parse trees using the parser (which we
// assume to be correct; hence the dependence on ParseStatementsTests)
// and then use a fold to convert them back to a string again, which
// should yield exactly the same result as what we inputted (provided
// we adhere to some conventions to do with spaces etc).
BOOST_DATA_TEST_CASE(test_fold_for_converting_to_str,
	boost::unit_test::data::make(stmts), stmt)
{
	// firstly, parse the statement, to get its parse tree, assuming
	// the parser is correct:
	std::stringstream s;
	s << stmt;
	auto result = parse_statements(s);
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto p_root = result.get().front();

	auto str_result = fold_parse_tree<std::string>(&eq_to_str,
		&identifier_to_str, p_root);

	BOOST_TEST(str_result == stmt);
}


BOOST_AUTO_TEST_SUITE_END();  // ParseTreeFoldTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


std::string eq_to_str(std::string lhs, std::string rhs)
{
	return lhs + " = " + rhs;
}


std::string identifier_to_str(std::string name,
	std::vector<std::string>::iterator arg_begin,
	std::vector<std::string>::iterator arg_end)
{
	if (arg_begin == arg_end)
	{
		return name;
	}
	else
	{
		return name + '(' + boost::algorithm::join(
			boost::make_iterator_range(arg_begin, arg_end),
			", ") + ')';
	}
}


