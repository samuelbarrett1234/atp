/*

SyntaxTreeFreeVarTests.cpp

Test the syntax trees assign valid values to the free variable IDs.

*/


#include <sstream>
#include <string>
#include <vector>
#include <boost/phoenix.hpp>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxTreeFold.h>
#include "../Test.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
using atp::logic::equational::fold_syntax_tree;
namespace phx = boost::phoenix;
using namespace phx::arg_names;


struct SyntaxTreeFreeVarTestsFixture
{
	SyntaxTreeFreeVarTestsFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
	}

	std::stringstream s;
	KnowledgeKernel ker;
};


// here are some statements, with plenty of free variables,
// for us to test their IDs:
static std::string stmts_with_free_vars[] =
{
	"*(x, *(y, z)) = *(*(x, y), z)",
	"*(x, y) = z",
	"*(*(x, y), *(z, w)) = i(i(i(i(*(x, w)))))"
};
static size_t num_free_vars_in_stmts[] =
{
	3, 3, 4
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SyntaxTreeFreeVarTests,
	SyntaxTreeFreeVarTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests"));


BOOST_DATA_TEST_CASE(test_free_var_ids_are_valid,
	boost::unit_test::data::make(stmts_with_free_vars)
	^ boost::unit_test::data::make(num_free_vars_in_stmts),
	stmt, num_free_vars)
{
	s << stmt;
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto p_node = ptree_to_stree(
		result.get().front(), ker);

	// should've been successful
	BOOST_REQUIRE(p_node != nullptr);

	std::vector<bool> exists_free;
	exists_free.resize(num_free_vars, false);

	auto eq_func = arg1 && arg2;
	auto const_func = phx::val(true);
	auto func_func = [](size_t, std::list<bool>::iterator begin,
		std::list<bool>::iterator end) -> bool
	{
		return std::all_of(begin, end, arg1);
	};

	auto free_func = [&exists_free, num_free_vars](size_t id) -> bool
	{
		if (id >= num_free_vars)
			return false;
		else
		{
			exists_free[id] = true;
			return true;
		}
	};

	BOOST_TEST(fold_syntax_tree<bool>(eq_func, free_func, const_func,
		func_func, p_node));
	BOOST_TEST(std::all_of(exists_free.begin(), exists_free.end(),
		arg1));
}


BOOST_AUTO_TEST_SUITE_END();  // ParseTreeToSyntaxTreeTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


