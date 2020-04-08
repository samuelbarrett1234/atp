/*

SemanticsHelperTests.cpp

This suite tests the functions found in SemanticsHelper.h /
SemanticsHelper.cpp, which are supporting functions for those
found in Semantics.h / Semantics.cpp. As a result, this suite
is a dependency for the corresponding SemanticsTests suite.

*/


#include <boost/bind.hpp>
#include <Internal/Equational/SemanticsHelper.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/StatementArray.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"


using atp::logic::equational::Statement;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::StatementArray;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
namespace semantics = atp::logic::equational::semantics;


struct SemanticsHelperTestsFixture
{
	SemanticsHelperTestsFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
		s << std::noskipws;

		// define group theory rules
		s << "x = *(e, x)\n";
		s << "x = *(x, e)\n";
		s << "e = *(x, i(x))\n";
		s << "e = *(i(x), x)\n";
		s << "*(x, *(y, z)) = *(*(x, y), z)";
		auto parse_trees = parse_statements(s);
		p_rules = std::make_shared<std::vector<Statement>>();
		for (auto ptree : parse_trees.get())
		{
			auto stree = ptree_to_stree(ptree,
				ker);
			p_rules->emplace_back(ker, stree);
		}
		ker.define_eq_rules(
			std::make_shared<StatementArray>(p_rules));

		// reset this
		s = std::stringstream();
		s << std::noskipws;
	}

	std::stringstream s;
	KnowledgeKernel ker;
	std::shared_ptr<std::vector<Statement>> p_rules;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SemanticsHelperTests,
	SemanticsHelperTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests"));


BOOST_AUTO_TEST_CASE(test_syntax_tree_identical)
{
	BOOST_TEST(false);
}


BOOST_AUTO_TEST_CASE(test_get_statement_sides)
{
	BOOST_TEST(false);
}


BOOST_AUTO_TEST_CASE(test_get_free_var_ids)
{
	BOOST_TEST(false);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_statement_sides"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
BOOST_AUTO_TEST_CASE(test_try_build_map)
{
	BOOST_TEST(false);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_syntax_tree_identical"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_statement_sides"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_get_free_var_ids"))
BOOST_AUTO_TEST_CASE(test_substitute_tree)
{
	BOOST_TEST(false);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_try_build_map"))
BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/SemanticsHelperTests/test_substitute_tree"))
BOOST_DATA_TEST_CASE(test_immediate_applications,
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"*(e, x) = x",
		"e = x",
		"*(*(x, y), z) = x" }) ^
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"*(*(e, x), e) = x",
		"*(x, i(x)) = x",
		"*(x, *(y, z)) = x" }),
	stmt, stmt_imm_app)
{
	s << stmt << "\n" << stmt_imm_app;

	auto ptrees = parse_statements(s);

	BOOST_REQUIRE(ptrees.has_value());
	BOOST_REQUIRE(ptrees->size() == 2);

	auto stmt_stree = ptree_to_stree(ptrees->front(), ker);
	auto stmt_imm_app_stree = ptree_to_stree(ptrees->back(), ker);

	// get LHSs

	auto stmt_lhs = dynamic_cast<EqSyntaxNode*>(
		stmt_stree.get())->left();
	auto stmt_imm_app_lhs = dynamic_cast<EqSyntaxNode*>(
		stmt_imm_app_stree.get())->left();

	// create substitution info

	semantics::SubstitutionInfo sub_info(ker,
		*p_rules,
		semantics::get_free_var_ids(stmt_stree));

	// get test immediate applications

	auto imm_apps = semantics::immediate_applications(stmt_lhs,
		sub_info);

	std::vector<std::string> strs;
	for (auto i : imm_apps)
		strs.push_back(syntax_tree_to_str(ker, i));

	// now test that stmt_imm_app_lhs appeared as a result

	BOOST_TEST(std::any_of(imm_apps.begin(), imm_apps.end(),
		boost::bind(&semantics::syntax_tree_identical,
			stmt_imm_app_lhs, _1)));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


