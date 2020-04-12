/**

\file

\author

\details This file tests the equational::Statement class. Its main
    job is to test the two "fold" functions in the class, because
    these are used as the building blocks of virtually every function
    in the equational::semantics namespace.

*/


#include <sstream>
#include <boost/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/Semantics.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::equational::ModelContext;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::SyntaxNodePtr;
namespace semantics = atp::logic::equational::semantics;
namespace phx = boost::phoenix;
namespace phxargs = phx::arg_names;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StatementTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
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
	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	BOOST_TEST(stmt_obj.to_str() == stmt);
}


BOOST_DATA_TEST_CASE(num_free_vars_test,
	boost::unit_test::data::make({ "x0 = x0",
		"*(x0, x1) = x1", "e = e", "x0 = e"}) ^
	boost::unit_test::data::make({ 1, 2, 0, 1 }),
	stmt, num_free_vars)
{
	s << stmt;
	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

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
	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	auto eq_to_str = phxargs::arg1 + " = " + phxargs::arg2;
	auto free_to_str = [](size_t id)
	{ return "x" + boost::lexical_cast<std::string>(id); };
	auto const_to_str = boost::bind(&ModelContext::symbol_name,
		boost::ref(ctx), _1);
	auto func_to_str = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return ctx.symbol_name(symb_id) + '(' + boost::algorithm::join(
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
	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	auto eq_to_str = phxargs::arg1 + " = " + phxargs::arg2;
	auto free_to_str = [](size_t id)
	{ return "x" + boost::lexical_cast<std::string>(id); };
	auto const_to_str = boost::bind(&ModelContext::symbol_name,
		boost::ref(ctx), _1);
	auto func_to_str = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return ctx.symbol_name(symb_id) + '(' + boost::algorithm::join(
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
	// for this test I want to introduce an additional constant
	// thus we need a different model context
	ctx_in = std::stringstream();
	ctx_in << "{\n"
		"	name : \"Group Theory\",\n"
		"definitions : [\n"
		"{ name: \"e\", arity : 0 },\n"
		"{ name: \"pi\", arity : 0 },\n"  // the new constant
		"{ name: \"i\", arity : 1 },\n"
		"{ name: \"*\", arity : 2 },\n"
		"], \n"
		"axioms : [ \n"
		"\"*(*(x, y), z) = *(x, *(y, z))\",\n"
		"\"*(x, e) = x\",\n"
		"\"*(e, x) = x\",\n"
		"\"*(x, i(x)) = e\",\n"
		"\"*(i(x), x) = e\",\n"
		"] }"
		;
	p_ctx = lang.try_create_context(ctx_in);
	const ModelContext& ctx2 = dynamic_cast<
		const ModelContext&>(*p_ctx);

	s << "i(x) = e \n";
	s << "*(x, y) = pi";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1 = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2 = dynamic_cast<const Statement&>(p_stmts->at(1));

	auto eq_func = phxargs::arg1 && phxargs::arg2;

	// there should be no free variable comparisons in this example
	// pair fold:
	auto free_func = phx::val(false);

	auto const_func = [ctx2](size_t id_lhs, size_t id_rhs)
	{
		return (ctx2.symbol_name(id_lhs) == "e" &&
			ctx2.symbol_name(id_rhs) == "pi");
	};

	// when comparing two functions with different arities, it should
	// use the default comparer
	auto f_func = phx::val(false);

	auto default_func = [ctx2](SyntaxNodePtr lhs, SyntaxNodePtr rhs)
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

		if (ctx2.symbol_name(p_lhs->get_symbol_id()) != "i")
			return false;
		if (ctx2.symbol_name(p_rhs->get_symbol_id()) != "*")
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

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1 = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2 = dynamic_cast<const Statement&>(p_stmts->at(1));

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
		BOOST_TEST(ctx.symbol_arity(id1) == 0);
		BOOST_TEST(ctx.symbol_arity(id2) == 0);

		return (id1 == id2) ? ctx.symbol_name(id1)
			: ("[" + ctx.symbol_name(id1) + ", " +
				ctx.symbol_name(id2) + "]");;
	};
	auto f_func = [this](size_t id1, size_t id2,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end)
	{
		// might as well test the arity here:
		BOOST_TEST(std::distance(begin, end)
			== ctx.symbol_arity(id1));
		BOOST_TEST(std::distance(begin, end)
			== ctx.symbol_arity(id2));

		auto str_id = (id1 == id2) ? ctx.symbol_name(id1)
			: ("[" + ctx.symbol_name(id1) + ", " +
				ctx.symbol_name(id2) + "]");
		return str_id + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end),
				", ") + ')';
	};
	auto default_func = [this](SyntaxNodePtr p_left,
		SyntaxNodePtr p_right)
	{
		return "[" + syntax_tree_to_str(ctx, p_left)
			+ ", " + syntax_tree_to_str(ctx, p_right) + "]";
	};

	auto str_result = stmt1.fold_pair<std::string>(eq_func,
		free_func, const_func, f_func, default_func,
		stmt2);

	BOOST_TEST((str_result ==
		"[*(x0, x1), e] = i(*(i(x0), i(x1)))" ||
		str_result == "[*(x1, x0), e] = i(*(i(x1), i(x0)))"));
}


BOOST_AUTO_TEST_CASE(test_adjoin_rhs)
{
	// stmt1:
	s << "*(x, y) = e\n";

	// stmt2:
	s << "i(x) = i(*(i(x), y))";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1 = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2 = dynamic_cast<const Statement&>(p_stmts->at(1));

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

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1 = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2 = dynamic_cast<const Statement&>(p_stmts->at(1));

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

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	auto sides = stmt.get_sides();

	BOOST_TEST(syntax_tree_to_str(ctx, sides.first)
		== side1);
	BOOST_TEST(syntax_tree_to_str(ctx, sides.second)
		== side2);
}


BOOST_DATA_TEST_CASE(test_map_free_vars,
	boost::unit_test::data::make({
		"*(x0, e) = x0",
		"*(x0, e) = *(e, x0)",
		"*(x1, x2) = x0" }) ^
	boost::unit_test::data::make({
		// (RHS is ignored here, only LHS is the substitution)
		"i(x0) = x0",
		"*(*(x0, x1), x2) = z",
		"x0 = x0" }) ^
	boost::unit_test::data::make({
		"*(i(x0), e) = i(x0)",
		"*(*(*(x0, x1), x2), e) = *(e, *(*(x0, x1), x2))",
		"*(x0, x0) = x0" }),
	original_str, substitution_lhs_str, target_str)
{
	s << original_str << "\n" << substitution_lhs_str << "\n"
		<< target_str;

	// load:

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto original_stmt = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto sub_stmt = dynamic_cast<const Statement&>(p_stmts->at(1));
	auto target_stmt = dynamic_cast<const Statement&>(p_stmts->at(2));

	// construct free var mapping:
	std::map<size_t, SyntaxNodePtr> free_var_map;
	for (auto id : original_stmt.free_var_ids())
	{
		// substitute all free variables for the LHS of the
		// substitution statement (as we said above, we will
		// ignore the RHS).
		free_var_map[id] = sub_stmt.get_sides().first;
	}

	// now test the function

	auto result_stmt = original_stmt.map_free_vars(free_var_map);

	BOOST_TEST(semantics::equivalent(result_stmt, target_stmt));

	// note: we need this check to make sure that, if the
	// substitution reduced the number of free variables, the set
	// has updated (because otherwise such an error would not be
	// caught.)
	BOOST_TEST(result_stmt.num_free_vars() ==
		target_stmt.num_free_vars());
}


BOOST_AUTO_TEST_CASE(
	test_extra_mapping_doesnt_fool_free_var_tracking)
{
	// test that if we apply a map which maps a free variable
	// "2" say, but 2 didn't exist in the first place, that the
	// resulting statement doesn't start tracking 2.

	s << "x = x";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	std::map<size_t, SyntaxNodePtr> free_var_map;

	// no matter what ID `stmt` assigned the free variable,
	// because there is only one of them, then this mapping
	// will surface the error we are testing for if it exists

	free_var_map[0] = FreeSyntaxNode::construct(1);
	free_var_map[1] = FreeSyntaxNode::construct(2);

	auto result = stmt.map_free_vars(free_var_map);

	BOOST_TEST(result.num_free_vars() == 1);
}


BOOST_AUTO_TEST_SUITE_END();  // StatementTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


