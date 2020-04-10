/*

StatementTests.cpp

This file tests the equational::Statement class. Its main job is to
test the two "fold" functions in the class, because these are used
as the building blocks of virtually every function in the
equational::semantics namespace.

*/


#include <sstream>
#include <boost/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/Semantics.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::StmtForm;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::SyntaxNodePtr;
namespace semantics = atp::logic::equational::semantics;
namespace phx = boost::phoenix;
namespace phxargs = phx::arg_names;


struct StatementTestsFixture
{
	std::stringstream s;
	KnowledgeKernel ker;

	StatementTestsFixture()
	{
		s << std::noskipws;

		// load group theory symbols:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StatementTests,
	StatementTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseDefinitionsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests"));


// only test this with one free variable due to naming issues
// of free variables
BOOST_DATA_TEST_CASE(test_statement_with_one_var_to_str,
	boost::unit_test::data::make({ "x0 = x0",
		"i(x0) = i(x0)", "e = e",
		"*(e, x0) = x0" }), stmt)
{
	// test that .to_str is the inverse of the parser

	s << stmt;
	auto result = parse_statements(s);
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);
	auto stree = ptree_to_stree(result.get().front(), ker);
	auto stmt_obj = Statement(ker, stree);

	BOOST_TEST(stmt_obj.to_str() == stmt);
}


BOOST_DATA_TEST_CASE(num_free_vars_test,
	boost::unit_test::data::make({ "x0 = x0",
		"*(x0, x1) = x1", "e = e", "x0 = e"}) ^
	boost::unit_test::data::make({ 1, 2, 0, 1 }),
	stmt, num_free_vars)
{
	s << stmt;
	auto result = parse_statements(s);
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);
	auto stree = ptree_to_stree(result.get().front(), ker);
	auto stmt_obj = Statement(ker, stree);

	BOOST_TEST(stmt_obj.num_free_vars()
		== num_free_vars);
}


BOOST_DATA_TEST_CASE(test_string_fold_is_inverse_to_parser,
	boost::unit_test::data::make({ "x0 = i(x0)",
		"*(x0, *(e, i(x0))) = *(x0, x0)",
		"*(x0, x1) = i(*(i(x0), i(x1)))",
		"*(i(x0), i(i(x0))) = e" }), stmt)
{
	// this is the best way I can think of testing the fold function;
	// by using it as a .to_str() function (of course, the Statement
	// interface already provides this, but we are doing this to test
	// the fold function specifically.)

	s << stmt;
	auto result = parse_statements(s);
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);
	auto stree = ptree_to_stree(result.get().front(), ker);
	auto stmt_obj = Statement(ker, stree);

	auto eq_to_str = phxargs::arg1 + " = " + phxargs::arg2;
	auto free_to_str = [](size_t id)
	{ return "x" + boost::lexical_cast<std::string>(id); };
	auto const_to_str = boost::bind(&KnowledgeKernel::symbol_name,
		boost::ref(ker), _1);
	auto func_to_str = [this](size_t symb_id,
		std::list<std::string>::iterator begin,
		std::list<std::string>::iterator end) -> std::string
	{
		return ker.symbol_name(symb_id) + '(' + boost::algorithm::join(
			boost::make_iterator_range(begin, end), ", "
		) + ')';
	};

	BOOST_TEST(stmt_obj.fold<std::string>(eq_to_str, free_to_str,
		const_to_str, func_to_str) == stmt);
}


BOOST_AUTO_TEST_CASE(
	test_string_fold_is_inverse_to_parser_for_two_free_vars)
{
	// this is the best way I can think of testing the fold function;
	// by using it as a .to_str() function (of course, the Statement
	// interface already provides this, but we are doing this to test
	// the fold function specifically.)

	s << "*(x0, x1) = i(*(i(x0), i(x1)))";
	auto result = parse_statements(s);
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);
	auto stree = ptree_to_stree(result.get().front(), ker);
	auto stmt_obj = Statement(ker, stree);

	auto eq_to_str = phxargs::arg1 + " = " + phxargs::arg2;
	auto free_to_str = [](size_t id)
	{ return "x" + boost::lexical_cast<std::string>(id); };
	auto const_to_str = boost::bind(&KnowledgeKernel::symbol_name,
		boost::ref(ker), _1);
	auto func_to_str = [this](size_t symb_id,
		std::list<std::string>::iterator begin,
		std::list<std::string>::iterator end) -> std::string
	{
		return ker.symbol_name(symb_id) + '(' + boost::algorithm::join(
			boost::make_iterator_range(begin, end), ", "
		) + ')';
	};

	auto str_result = stmt_obj.fold<std::string>(
		eq_to_str, free_to_str,
		const_to_str, func_to_str);

	BOOST_TEST((str_result == "*(x0, x1) = i(*(i(x0), i(x1)))" ||
		str_result == "*(x1, x0) = i(*(i(x1), i(x0)))"));
}


BOOST_AUTO_TEST_CASE(test_pair_fold_with_conflicting_func_arities)
{
	// define some extra symbol for this test
	ker.define_symbol("pi", 0);

	s << "i(x) = e \n";
	s << "*(x, y) = pi";

	auto results = parse_statements(s);

	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto stree1 = ptree_to_stree(results.get().front(), ker);
	auto stree2 = ptree_to_stree(results.get().back(), ker);

	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);

	auto eq_func = phxargs::arg1 && phxargs::arg2;

	// there should be no free variable comparisons in this example
	// pair fold:
	auto free_func = phx::val(false);

	auto const_func = [this](size_t id_lhs, size_t id_rhs)
	{
		return (ker.symbol_name(id_lhs) == "e" &&
			ker.symbol_name(id_rhs) == "pi");
	};

	// when comparing two functions with different arities, it should
	// use the default comparer
	auto f_func = phx::val(false);

	auto default_func = [this](SyntaxNodePtr lhs, SyntaxNodePtr rhs)
		-> bool
	{
		if (lhs->get_type() != SyntaxNodeType::FUNC ||
			rhs->get_type() != SyntaxNodeType::FUNC)
			return false;
		auto p_lhs = dynamic_cast<FuncSyntaxNode*>(lhs.get());
		auto p_rhs = dynamic_cast<FuncSyntaxNode*>(rhs.get());

		if (p_lhs == nullptr ||
			p_rhs == nullptr)
			return false;

		if (ker.symbol_name(p_lhs->get_symbol_id()) != "i")
			return false;
		if (ker.symbol_name(p_rhs->get_symbol_id()) != "*")
			return false;

		return true;  // passed the tests!
	};

	BOOST_TEST(stmt1.fold_pair<bool>(eq_func, free_func, const_func,
		f_func, default_func, stmt2));
}


BOOST_AUTO_TEST_CASE(test_pair_fold_by_converting_pair_to_str)
{
	s << "*(x, y) = i(*(i(x), i(y))) \n";
	s << "e = i(*(i(x), i(y)))";

	auto results = parse_statements(s);

	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto stree1 = ptree_to_stree(results.get().front(), ker);
	auto stree2 = ptree_to_stree(results.get().back(), ker);

	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);

	auto eq_func = phxargs::arg1 + " = " + phxargs::arg2;
	auto free_func = [](size_t id1, size_t id2)
	{
		return (id1 == id2) ? 
			("x" + boost::lexical_cast<std::string>(id1)) :
			("x[" + boost::lexical_cast<std::string>(id1)
			+ ", " + boost::lexical_cast<std::string>(id2) + "]");
	};
	auto const_func = [this](size_t id1, size_t id2)
	{
		// might as well test the arity here:
		BOOST_TEST(ker.symbol_arity_from_id(id1) == 0);
		BOOST_TEST(ker.symbol_arity_from_id(id2) == 0);

		return (id1 == id2) ? ker.symbol_name(id1)
			: ("[" + ker.symbol_name(id1) + ", " +
				ker.symbol_name(id2) + "]");;
	};
	auto f_func = [this](size_t id1, size_t id2,
		std::list<std::string>::iterator begin,
		std::list<std::string>::iterator end)
	{
		// might as well test the arity here:
		BOOST_TEST(std::distance(begin, end)
			== ker.symbol_arity_from_id(id1));
		BOOST_TEST(std::distance(begin, end)
			== ker.symbol_arity_from_id(id2));

		auto str_id = (id1 == id2) ? ker.symbol_name(id1)
			: ("[" + ker.symbol_name(id1) + ", " +
				ker.symbol_name(id2) + "]");
		return str_id + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end),
				", ") + ')';
	};
	auto default_func = [this](SyntaxNodePtr p_left,
		SyntaxNodePtr p_right)
	{
		return "[" + syntax_tree_to_str(ker, p_left)
			+ ", " + syntax_tree_to_str(ker, p_right) + "]";
	};

	auto str_result = stmt1.fold_pair<std::string>(eq_func,
		free_func, const_func, f_func, default_func,
		stmt2);

	BOOST_TEST((str_result ==
		"[*(x0, x1), e] = i(*(i(x0), i(x1)))" ||
		str_result == "[*(x1, x0), e] = i(*(i(x1), i(x0)))"));
}


BOOST_AUTO_TEST_CASE(test_adjoint_rhs)
{
	// stmt1:
	s << "*(x, y) = e\n";

	// stmt2:
	s << "i(x) = i(*(i(x), y))";

	auto results = parse_statements(s);

	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto stree1 = ptree_to_stree(results->front(), ker);
	auto stree2 = ptree_to_stree(results->back(), ker);

	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);

	auto stmt_adjoin_result = stmt1.adjoin_rhs(stmt2);

	auto adjoin_as_str = stmt_adjoin_result.to_str();

	BOOST_TEST((adjoin_as_str ==
		"*(x0, x1) = i(*(i(x0), x1))" || adjoin_as_str
		== "*(x1, x0) = i(*(i(x1), x0))"));
}


BOOST_DATA_TEST_CASE(test_transpose,
	boost::unit_test::data::make({ "x0 = i(x0)",
		"i(x0) = *(x0, x1)", "*(x0, x0) = i(*(x0, x0))",
		"*( i(x), i(i(x)) ) = e" }) ^
	boost::unit_test::data::make({
		"i(x0) = x0", "*(x0, x1) = i(x0)",
		"i(*(x0, x0)) = *(x0, x0)",
		"e = *( i(x), i(i(x)) )" }),
		original, target)
{
	// create statements from the "original" and
	// "target" strings
	s << original << "\n" << target;
	auto results = parse_statements(s);
	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);
	auto stree1 = ptree_to_stree(results.get().front(), ker);
	auto stree2 = ptree_to_stree(results.get().back(), ker);
	BOOST_REQUIRE(stree1 != nullptr);
	BOOST_REQUIRE(stree2 != nullptr);
	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);
	// check that the transpose of one of them is identical
	// to the other one
	BOOST_TEST(semantics::identical(stmt1,
		stmt2.transpose()));
	BOOST_TEST(semantics::identical(stmt2,
		stmt1.transpose()));
}


// the data for the test case below (these will appear as the
// different sides of the equation)
const char* stmt_sides_data[] =
{
	"x0", "e", "i(x0)",
	"*(x0, x0)", "*(x0, *(x0, x0))",
	"*(i(x0), i(x0))", "i(*(x0, x0))"
};
BOOST_DATA_TEST_CASE(test_get_statement_sides,
	boost::unit_test::data::make(stmt_sides_data)
	* boost::unit_test::data::make(stmt_sides_data),
	side1, side2)
{
	s << side1 << " = " << side2;
	auto ptree = parse_statements(s);
	BOOST_REQUIRE(ptree.has_value());
	BOOST_REQUIRE(ptree->size() == 1);
	auto stree = ptree_to_stree(ptree->front(), ker);
	auto stmt = Statement(ker, stree);

	auto sides = stmt.get_sides();

	BOOST_TEST(syntax_tree_to_str(ker, sides.first)
		== side1);
	BOOST_TEST(syntax_tree_to_str(ker, sides.second)
		== side2);
}


BOOST_AUTO_TEST_SUITE_END();  // StatementTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


