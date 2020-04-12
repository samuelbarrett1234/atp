/**

\file

\author Samuel Barrett

\details This suite tests the functions found in
    SemanticsHelper.h/SemanticsHelper.cpp, which are supporting
	functions for those found in Semantics.h/Semantics.cpp. As a
	result, this suite is a dependency for the corresponding
	SemanticsTests suite.

*/


#include <boost/bind.hpp>
#include <Internal/Equational/SemanticsHelper.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/StatementArray.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"
#include "StandardTestFixture.h"


using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::StatementArray;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
namespace semantics = atp::logic::equational::semantics;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SemanticsHelperTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests"));


// the data for the test case below (using the * cartesian
// product operator, we will consider all pairs of statements
// in this list.)
const char* stree_identical_stmt_data[] =
{
	"x = x", "e = e",
	"i(x) = x", "*(x, y) = *(y, x)",
	"*(i(y), i(x)) = i(*(x, y))",
	"*(*(x, y), z) = *(x, *(y, z))",
	"y = y", "x = e", "i(x) = y",
	"z = i(y)", "z = i(z)"
};
BOOST_DATA_TEST_CASE(test_syntax_tree_identical,
	boost::unit_test::data::make(stree_identical_stmt_data)
	* boost::unit_test::data::make(stree_identical_stmt_data),
	stmt1, stmt2)
{
	s << stmt1 << '\n' << stmt2;

	auto ptrees = parse_statements(s);
	BOOST_REQUIRE(ptrees.has_value());
	BOOST_REQUIRE(ptrees->size() == 2);

	auto stmt1_stree = ptree_to_stree(ptrees->front(), ctx);
	auto stmt2_stree = ptree_to_stree(ptrees->back(), ctx);

	// convert the two statements to string (don't use stmt1
	// and stmt2, because the free variable names might be
	// different. The syntax trees should be identical iff
	// `syntax_tree_to_str` returns the same result, but that
	// does not hold for the string representations `stmt1`
	// and `stmt2` in general.)
	auto stmt1_str = syntax_tree_to_str(ctx, stmt1_stree);
	auto stmt2_str = syntax_tree_to_str(ctx, stmt2_stree);

	BOOST_TEST(semantics::syntax_tree_identical(stmt1_stree,
		stmt2_stree) == (stmt1_str == stmt2_str));
}


BOOST_DATA_TEST_CASE(test_get_free_var_ids,
	boost::unit_test::data::make({
		"*(x, y) = *(y, x)",
		"*(*(x, y), z) = *(x, *(y, z))",
		"i(i(x)) = e" }) ^ boost::unit_test::data::make({
		2, 3, 1 }), stmt_txt, num_free)
{
	s << stmt_txt;
	auto ptree = parse_statements(s);
	BOOST_REQUIRE(ptree.has_value());
	BOOST_REQUIRE(ptree->size() == 1);
	auto stree = ptree_to_stree(ptree->front(), ctx);
	
	// use testing the size of the ID set returned as a proxy
	// for checking that the actual IDs returned are correct
	// (rational: I don't want to write another fold, and checking
	// the size probably suffices to detect off-by-one errors!)

	BOOST_TEST(semantics::get_free_var_ids(stree).size()
		== num_free);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
BOOST_AUTO_TEST_CASE(test_try_build_map_positive)
{
	// here we will extract the two sides of this equation separately
	// we will test to see if there exists a substitution for the LHS
	// which makes the RHS.
	s << "i(*(x, y)) = i(*(i(z), i(x)))";

	auto ptree = parse_statements(s);
	BOOST_REQUIRE(ptree.has_value());
	BOOST_REQUIRE(ptree->size() == 1);

	auto stree = ptree_to_stree(ptree->front(), ctx);

	// get LHS and RHS of the syntax tree we just built
	auto sides = Statement(ctx, stree).get_sides();

	auto mapping = semantics::try_build_map(sides.first,
		sides.second);

	// this is a successful example
	BOOST_REQUIRE(mapping.has_value());

	BOOST_TEST(mapping->size() == 2);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
	BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
		"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
	BOOST_AUTO_TEST_CASE(test_try_build_map_negative)
{
	// here we will extract the two sides of this equation separately
	// we will test to see if there does not exist a substitution
	// for the LHS which makes the RHS.
	s << "i(*(e, y)) = i(*(i(z), i(x)))";

	auto ptree = parse_statements(s);
	BOOST_REQUIRE(ptree.has_value());
	BOOST_REQUIRE(ptree->size() == 1);

	auto stree = ptree_to_stree(ptree->front(), ctx);

	// get LHS and RHS of the syntax tree we just built
	auto sides = Statement(ctx, stree).get_sides();

	auto mapping = semantics::try_build_map(sides.first,
		sides.second);

	// this is a negative example
	BOOST_TEST(!mapping.has_value());
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
	BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
		"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
	BOOST_AUTO_TEST_CASE(test_try_build_map_empty)
{
	// check the case where there exists a free variable mapping,
	// but where that mapping is empty

	// here we will extract the two sides of this equation separately
	// we will test to see if there exists a substitution for the LHS
	// which makes the RHS.
	s << "e = e";

	auto ptree = parse_statements(s);
	BOOST_REQUIRE(ptree.has_value());
	BOOST_REQUIRE(ptree->size() == 1);

	auto stree = ptree_to_stree(ptree->front(), ctx);

	// get LHS and RHS of the syntax tree we just built
	auto sides = Statement(ctx, stree).get_sides();

	auto mapping = semantics::try_build_map(sides.first,
		sides.second);

	// this is a successful example
	BOOST_REQUIRE(mapping.has_value());

	BOOST_TEST(mapping->empty());
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
BOOST_AUTO_TEST_CASE(test_substitute_tree_with_new_free_var)
{
	// use the second statement to substitute into the first
	// statement; check that the introduction of a new free
	// variable is handled correctly
	s << "x = e \n e = *(x, i(x))";

	auto ptrees = parse_statements(s);
	BOOST_REQUIRE(ptrees.has_value());
	BOOST_REQUIRE(ptrees->size() == 2);

	auto premise_stree = ptree_to_stree(ptrees->front(), ctx);
	auto rule_stree = ptree_to_stree(ptrees->back(), ctx);

	auto rule_sides = Statement(ctx, rule_stree).get_sides();

	semantics::SubstitutionInfo sub_info(ker,
		{ Statement(ctx, rule_stree) },
		semantics::get_free_var_ids(premise_stree) );

	auto sub_tree_results = semantics::substitute_tree(
		rule_sides.second, sub_info,
		// empty mapping:
		std::map<size_t, SyntaxNodePtr>(),
		// first rule (out of a rule array of size 1):
		0);

	BOOST_TEST(sub_tree_results.size() == 4);

	// convert results to strings to make it easier to check
	// if the right results were obtained:
	std::list<std::string> results_as_str;
	for (auto p_result : sub_tree_results)
		results_as_str.push_back(syntax_tree_to_str(
			ctx, p_result));

	// check the substitution returned each of the results
	// we were expecting:

	BOOST_TEST((std::find(results_as_str.begin(),
		results_as_str.end(), "*(e, i(e))")
		!= results_as_str.end()));

	BOOST_TEST((std::find(results_as_str.begin(),
		results_as_str.end(), "*(x0, i(x0))")
		!= results_as_str.end()));

	BOOST_TEST((std::find(results_as_str.begin(),
		results_as_str.end(), "*(x0, i(e))")
		!= results_as_str.end()));

	BOOST_TEST((std::find(results_as_str.begin(),
		results_as_str.end(), "*(e, i(x0))")
		!= results_as_str.end()));
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_try_build_map_positive"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_try_build_map_negative"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_try_build_map_empty"))
BOOST_DATA_TEST_CASE(test_immediate_applications,
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"*(e, x) = x",
		"e = x",
		"*(*(x, y), z) = x",
		"*(*(x, x), x) = x" }) ^
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"*(*(e, x), e) = x",
		"*(x, i(x)) = x",
		"*(x, *(y, z)) = x",
		"*(x, *(x, x)) = x" }),
	stmt, stmt_imm_app)
{
	s << stmt << "\n" << stmt_imm_app;

	auto ptrees = parse_statements(s);

	BOOST_REQUIRE(ptrees.has_value());
	BOOST_REQUIRE(ptrees->size() == 2);

	auto stmt_stree = ptree_to_stree(ptrees->front(), ctx);
	auto stmt_imm_app_stree = ptree_to_stree(ptrees->back(), ctx);

	// get LHSs

	auto stmt_lhs = dynamic_cast<EqSyntaxNode*>(
		stmt_stree.get())->left();
	auto stmt_imm_app_lhs = dynamic_cast<EqSyntaxNode*>(
		stmt_imm_app_stree.get())->left();

	// create substitution info

	semantics::SubstitutionInfo sub_info(ctx,
		*p_rules,
		semantics::get_free_var_ids(stmt_stree));

	// get test immediate applications

	auto imm_apps = semantics::immediate_applications(stmt_lhs,
		sub_info);

	std::vector<std::string> strs;
	for (auto i : imm_apps)
		strs.push_back(syntax_tree_to_str(ctx, i));

	// now test that stmt_imm_app_lhs appeared as a result

	BOOST_TEST(std::any_of(imm_apps.begin(), imm_apps.end(),
		boost::bind(&semantics::syntax_tree_identical,
			stmt_imm_app_lhs, _1)));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


