/**

\file

\author

\details This file tests the equational::Statement class.

*/


#include <sstream>
#include <boost/phoenix.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/SyntaxNodes.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::equational::ModelContext;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::Expression;
namespace phx = boost::phoenix;
namespace phxargs = phx::arg_names;


// apply the following map to free variable IDs (IDs not present
// in the map are left unchanged - so for example, were the map to
// be empty, this would be the identity function.)
static Statement map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	Statement stmt);


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StatementTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ExpressionTests"));


// the data for the test case below (using the * cartesian
// product operator, we will consider all pairs of statements
// in this list.)
const char* identical_stmt_data[] =
{
	"x = x", "e = e",
	"i(x) = x", "*(x, y) = *(y, x)",
	"*(i(y), i(x)) = i(*(x, y))",
	"*(*(x, y), z) = *(x, *(y, z))",
	"x = e", "i(x) = y", "z = i(z)"
};
BOOST_DATA_TEST_CASE(test_identical,
	boost::unit_test::data::make(identical_stmt_data)
	* boost::unit_test::data::make(identical_stmt_data),
	stmt1_str, stmt2_str)
{
	s << stmt1_str << '\n' << stmt2_str;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	const auto& stmt1 = dynamic_cast<const Statement&>(
		p_stmts->at(0));
	const auto& stmt2 = dynamic_cast<const Statement&>(
		p_stmts->at(1));

	// test returns the correct value
	BOOST_TEST(stmt1.identical(stmt2) == (stmt1_str == stmt2_str));

	// test symmetric
	BOOST_TEST(stmt1.identical(stmt2) == stmt2.identical(stmt1));

	// test reflexive
	BOOST_TEST(stmt1.identical(stmt1));
}


BOOST_AUTO_TEST_CASE(test_dependent_sides_parsing)
{
	// test that the loading of a statement takes into account
	// interactions between the two sides of the statement
	s << "*(x, y) = *(y, x)\n";
	s << "*(x, y) = *(x, y)";

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	const auto& stmt1 = dynamic_cast<const Statement&>(
		p_stmts->at(0));
	const auto& stmt2 = dynamic_cast<const Statement&>(
		p_stmts->at(1));

	BOOST_TEST(!stmt1.equivalent(stmt2));
	BOOST_TEST(!stmt1.identical(stmt2));
}


BOOST_DATA_TEST_CASE(test_num_free_vars,
	boost::unit_test::data::make({
		"*(x, y) = *(y, x)",
		"*(*(x, y), z) = *(x, *(y, z))",
		"i(i(x)) = e" }) ^ boost::unit_test::data::make({
		2, 3, 1 }), stmt_txt, num_free)
{
	s << stmt_txt;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	const auto& stmt1 = dynamic_cast<const Statement&>(
		p_stmts->at(0));

	BOOST_TEST(stmt1.num_free_vars() == num_free);
}


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
		"	\"name\" : \"Group Theory\",\n"
		"\"definitions\" : [\n"
		"{ \"name\": \"e\", \"arity\" : 0 },\n"
		"{ \"name\": \"pi\", \"arity\" : 0 },\n"  // the new constant
		"{ \"name\": \"i\", \"arity\" : 1 },\n"
		"{ \"name\": \"*\", \"arity\" : 2 }\n"
		"], \n"
		"\"axioms\" : [ \n"
		"\"*(*(x, y), z) = *(x, *(y, z))\",\n"
		"\"*(x, e) = x\",\n"
		"\"*(e, x) = x\",\n"
		"\"*(x, i(x)) = e\",\n"
		"\"*(i(x), x) = e\"\n"
		"] }"
		;
	p_ctx = lang.try_create_context(ctx_in);
	const ModelContext& ctx2 = dynamic_cast<
		const ModelContext&>(*p_ctx);

	s << "i(x) = e \n";
	s << "*(x, y) = pi";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx2);
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

	auto default_func = [ctx2](Expression lhs, Expression rhs)
		-> bool
	{
		if (lhs.root_type() != SyntaxNodeType::FUNC ||
			rhs.root_type() != SyntaxNodeType::FUNC)
			return false;

		if (ctx2.symbol_name(lhs.root_id()) != "i")
			return false;
		if (ctx2.symbol_name(rhs.root_id()) != "*")
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
	auto default_func = [this](Expression lhs,
		Expression rhs)
	{
		return "[" + lhs.to_str() + ", " + rhs.to_str() + "]";
	};

	auto str_result = stmt1.fold_pair<std::string>(eq_func,
		free_func, const_func, f_func, default_func,
		stmt2);

	BOOST_TEST((str_result ==
		"[*(x0, x1), e] = i(*(i(x0), i(x1)))" ||
		str_result == "[*(x1, x0), e] = i(*(i(x1), i(x0)))"));
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
	BOOST_TEST(stmt1.identical(stmt2.transpose()));
	BOOST_TEST(stmt2.identical(stmt1.transpose()));
	// also test their sides
	BOOST_TEST(stmt1.lhs().identical(stmt2.rhs()));
	BOOST_TEST(stmt2.lhs().identical(stmt1.rhs()));
}


// the data for the test case below (these will appear as the
// different sides of the equation)
const char* stmt_sides_data[] =
{
	"x0", "e", "i(x0)",
	"*(x0, x0)", "*(x0, *(x0, x0))",
	"*(i(x0), i(x0))", "i(*(x0, x0))"
};
BOOST_DATA_TEST_CASE(test_statement_lhs_rhs,
	boost::unit_test::data::make(stmt_sides_data)
	* boost::unit_test::data::make(stmt_sides_data),
	side1, side2)
{
	s << side1 << " = " << side2;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	BOOST_TEST(stmt.lhs().to_str() == side1);
	BOOST_TEST(stmt.rhs().to_str() == side2);
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
	std::map<size_t, Expression> free_var_map;
	for (auto id : original_stmt.free_var_ids())
	{
		// substitute all free variables for the LHS of the
		// substitution statement (as we said above, we will
		// ignore the RHS).
		free_var_map.insert(std::make_pair(id, sub_stmt.lhs()));
	}

	// now test the function

	auto result_stmt = original_stmt.map_free_vars(free_var_map);

	BOOST_TEST(result_stmt.equivalent(target_stmt));

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

	std::map<size_t, Expression> free_var_map;

	// no matter what ID `stmt` assigned the free variable,
	// because there is only one of them, then this mapping
	// will surface the error we are testing for if it exists

	free_var_map.insert({ 0, Expression(ctx,
		FreeSyntaxNode::construct(1)) });
	free_var_map.insert({ 1, Expression(ctx,
		FreeSyntaxNode::construct(2)) });

	auto result = stmt.map_free_vars(free_var_map);

	BOOST_TEST(result.num_free_vars() == 1);
}


BOOST_DATA_TEST_CASE(test_true_by_reflexivity_works_on_examples,
	boost::unit_test::data::make({
		// some statements to test on:
		"x=x", "x=y", "i(x)=x", "x=e", "e=e", "i(x)=i(x)",
		"*(x, y)=*(x, y)", "*(x, y)=*(y, x)" }) ^
	boost::unit_test::data::make({
		// whether or not those statements are symmetric in the
		// equals sign
		true, false, false, false, true, true, true, false }),
	stmt, is_symmetric)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	BOOST_TEST(stmt_obj.true_by_reflexivity() == is_symmetric);
}


// firstly we will test the `identical` and `equivalent` functions
// in scenarios where they both should return the same value:
BOOST_DATA_TEST_CASE(test_identical_and_equivalent,
	boost::unit_test::data::make({
		// first list of statements
		"x = x", "i(x) = x", "x = e", "e = e",
		"*(x, x) = *( i(x), i(i(x)) )" }) ^
	boost::unit_test::data::make({
		// second list of statements
		"y = y", "i(y) = y", "e = e", "e = e",
		"*(x, x) = *( x, i(x) )" }) ^
	boost::unit_test::data::make({
		// whether or not those statements are equivalent
		// (and also identical - it makes no difference here) and
		// note that "x=x" and "y=y" are identical because, once
		// the statements are parsed, both have 0 as their free
		// variable IDs.
		true, true, false, true, false }),
	stmt1, stmt2, is_equivalent_and_identical)
{
	// parse them both at the same time
	s << stmt1 << std::endl << stmt2;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1_obj = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2_obj = dynamic_cast<const Statement&>(p_stmts->at(1));

	// identical and equivalent
	// (try calling both ways round)

	BOOST_TEST(stmt1_obj.identical(stmt2_obj)
		== is_equivalent_and_identical);
	BOOST_TEST(stmt2_obj.identical(stmt1_obj)
		== is_equivalent_and_identical);

	BOOST_TEST(stmt1_obj.equivalent(stmt2_obj)
		== is_equivalent_and_identical);
	BOOST_TEST(stmt2_obj.equivalent(stmt1_obj)
		== is_equivalent_and_identical);
}


// now we will apply both functions to an example where the two
// statements are equivalent but not identical (we will obtain
// the second statement from the ones given in the list by
// remapping the free variable IDs.)
BOOST_DATA_TEST_CASE(test_equivalent_but_not_identical,
	boost::unit_test::data::make({
		"x = x", "i(x) = x", "x = e", "*(x, y) = x",
		"*(x, y) = i(*(x, y))" }), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	// change the free variable IDs so that they are no longer
	// identical
	std::map<size_t, size_t> free_id_map;
	free_id_map[0] = 1;  free_id_map[1] = 2;

	auto stmt2_obj = map_free_ids(free_id_map, stmt1_obj);

	// not identical, but equivalent
	// (try calling both ways round)

	BOOST_TEST(!stmt1_obj.identical(stmt2_obj));
	BOOST_TEST(!stmt2_obj.identical(stmt1_obj));

	BOOST_TEST(stmt1_obj.equivalent(stmt2_obj));
	BOOST_TEST(stmt2_obj.equivalent(stmt1_obj));
}


// finally we will consider cases in which the statements are
// neither equivalent nor identical (we will obtain
// the second statement from the ones given in the list by
// remapping the free variable IDs.)
BOOST_DATA_TEST_CASE(test_neither_equivalent_nor_identical,
	boost::unit_test::data::make({
		"x = y", "i(x) = y", "x = *(e, y)", "*(x, y) = x" }), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	auto ids = stmt1_obj.free_var_ids();

	BOOST_REQUIRE(ids.size() == 2);
	size_t id1 = *ids.begin();
	size_t id2 = *std::next(ids.begin());

	// replace one of the free variables with the other, to eliminate
	// one of them, and make the statement have one fewer free
	// variable (recall that: if two statements differ in their
	// number of free variables then they are neither equivalent nor
	// identical).
	std::map<size_t, size_t> free_id_map;
	free_id_map[id1] = id2;

	auto stmt2_obj = map_free_ids(free_id_map, stmt1_obj);

	// not identical nor equivalent
	// (try calling both ways round)

	BOOST_TEST(!stmt1_obj.identical(stmt2_obj));
	BOOST_TEST(!stmt2_obj.identical(stmt1_obj));

	BOOST_TEST(!stmt1_obj.equivalent(stmt2_obj));
	BOOST_TEST(!stmt2_obj.equivalent(stmt1_obj));
}


BOOST_AUTO_TEST_CASE(test_equivalent_invariant_to_reflection)
{
	// reflect the statement along the equals sign, should still
	// be equivalent but not identical
	s << "*(x, y) = i(*(x, y)) \n i(*(x, y)) = *(x, y)";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt1 = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto stmt2 = dynamic_cast<const Statement&>(p_stmts->at(1));

	BOOST_TEST(stmt1.equivalent(stmt2));
	BOOST_TEST(stmt2.equivalent(stmt1));
}


BOOST_DATA_TEST_CASE(test_implies,
	boost::unit_test::data::make({
		"*(x, i(x)) = e",
		"*(x, y) = *(y, x)",
		"x = x",
		"x = x",
		"x = i(x)",
		"*(*(x, y), z) = *(x, *(y, z))" }) ^
	boost::unit_test::data::make({
		"*(i(x), i(i(x))) = e",
		"*(x, i(x)) = *(i(x), x)",
		"e = e",
		"i(x) = i(x)",
		"i(e) = e",
		"*(x, *(i(x), x)) = *(*(x, i(x)), x)" }),
	premise_stmt, concl_stmt)
{
	s << premise_stmt << '\n' << concl_stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto premise = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto concl = dynamic_cast<const Statement&>(p_stmts->at(1));

	BOOST_TEST(premise.implies(concl));
}


BOOST_DATA_TEST_CASE(test_NOT_implies,
	boost::unit_test::data::make({
		"*(x, e) = x" }) ^
	boost::unit_test::data::make({
		"*(*(x, y), e) = *(y, x)" }),
	premise_stmt, concl_stmt)
{
	s << premise_stmt << '\n' << concl_stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto premise = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto concl = dynamic_cast<const Statement&>(p_stmts->at(1));

	BOOST_TEST(!premise.implies(concl));
}


BOOST_DATA_TEST_CASE(iterator_walking_test,
	boost::unit_test::data::make({
		"x0 = x0", "e = i(e)", "i(x0) = *(e, i(x0))",
		"*(*(x0, e), e) = x1" }) ^
	boost::unit_test::data::make({
		1, 1, 2, 4 }) ^
	boost::unit_test::data::make({
		"x0", "i(e)", "*(e, i(x0))", "e" }),
	stmt_str, advance_num, sub_expr_str)
{
	s << stmt_str << "\n";
	s << sub_expr_str << " = x0\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto original = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto sub = dynamic_cast<const Statement&>(p_stmts->at(1)).lhs();

	auto iter = original.begin();
	std::advance(iter, advance_num);

	BOOST_TEST(sub.equivalent(*iter));
	BOOST_TEST(iter->equivalent(sub));
}


BOOST_DATA_TEST_CASE(end_iterator_test,
	boost::unit_test::data::make({
		"x0 = x0", "e = i(e)", "i(x0) = *(e, i(x0))",
		"*(*(x0, e), e) = x1" }) ^
	boost::unit_test::data::make({
		2, 3, 6, 6 }),
	stmt_str, advance_num)
{
	s << stmt_str << "\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	auto iter = stmt.begin();

	BOOST_TEST((iter == stmt.begin()));

	std::advance(iter, advance_num);

	BOOST_TEST((iter == stmt.end()));
}


BOOST_DATA_TEST_CASE(replace_tests,
	boost::unit_test::data::make({
		// expressions to start with
		"x = x", "i(x) = *(x, e)",
		"x = *(*(x, e), e)" })
	^ boost::unit_test::data::make({
		// replacement
		"i(x)", "e", "i(x)" })
	^ boost::unit_test::data::make({
		// pre-order traversal index of the substitution
		// location
		0, 2, 3 })
	^ boost::unit_test::data::make({
		// result
		"i(x) = x", "i(x) = e",
		"x = *(*(i(x), e), e)" }),
	original_stmt, sub_expr, sub_idx, result_stmt)
{
	s << original_stmt << "\n";
	s << sub_expr << " = x\n";  // arbitrary RHS
	s << result_stmt << "\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto original = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto sub = dynamic_cast<const Statement&>(p_stmts->at(1)).lhs();
	auto correct_result = dynamic_cast<const Statement&>(p_stmts->at(2));

	auto iter = original.begin();
	std::advance(iter, sub_idx);
	auto result = original.replace(iter, sub);

	BOOST_TEST(correct_result.equivalent(result));
}


BOOST_AUTO_TEST_SUITE_END();  // StatementTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


Statement map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	Statement stmt)
{
	std::map<size_t, Expression> new_free_map;

	// change it so we map to expressions
	for (auto pair : free_id_map)
	{
		auto p_syntax_tree = FreeSyntaxNode::construct(pair.second);
		new_free_map.insert(std::make_pair(pair.first,
			Expression(stmt.context(), p_syntax_tree)));
	}

	// make the map total
	for (auto id : stmt.free_var_ids())
	{
		if (new_free_map.find(id) == new_free_map.end())
		{
			auto p_syntax_tree = FreeSyntaxNode::construct(id);
			new_free_map.insert(std::make_pair(id,
				Expression(stmt.context(), p_syntax_tree)));
		}
	}

	// utilise the stmt's helper function for this
	return stmt.map_free_vars(new_free_map);
}


