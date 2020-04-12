/**

\file

\author Samuel Barrett

\details This suite tests the functions that are found in the
    Semantics.h/Semantics.cpp files. Those files contain an array of
	helper functions and algorithms for dealing with syntax trees.

*/


#include <boost/bind.hpp>
#include <Internal/Equational/Semantics.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
namespace semantics = atp::logic::equational::semantics;


// apply the following map to free variable IDs (IDs not present
// in the map are left unchanged - so for example, were the map to
// be empty, this would be the identity function.)
static Statement map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	Statement stmt);


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SemanticsTests, StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SemanticsHelperTests"));


BOOST_DATA_TEST_CASE(test_true_by_reflexivity_works_on_examples,
	boost::unit_test::data::make({
		// some statements to test on:
		"x=x", "x=y", "i(x)=x", "x=e", "e=e", "i(x)=i(x)",
		"*(x, y)=*(x, y)", "*(x, y)=*(y, x)" }) ^
	boost::unit_test::data::make({
		// whether or not those statements are symmetric in the
		// equals sign
		true, false, false, false, true, true, true, false}),
	stmt, is_symmetric)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt_obj = dynamic_cast<const Statement&>(p_stmts->at(0));

	BOOST_TEST(semantics::true_by_reflexivity(stmt_obj)
		== is_symmetric);
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
	BOOST_TEST(semantics::identical(stmt1_obj,
		stmt2_obj) == is_equivalent_and_identical);
	BOOST_TEST(semantics::identical(stmt2_obj,
		stmt1_obj) == is_equivalent_and_identical);
	BOOST_TEST(semantics::equivalent(stmt1_obj,
		stmt2_obj) == is_equivalent_and_identical);
	BOOST_TEST(semantics::equivalent(stmt2_obj,
		stmt1_obj) == is_equivalent_and_identical);
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
	BOOST_TEST(!semantics::identical(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(!semantics::identical(stmt2_obj,
		stmt1_obj));
	BOOST_TEST(semantics::equivalent(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(semantics::equivalent(stmt2_obj,
		stmt1_obj));
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

	// map all used variables to ID 0!
	std::map<size_t, size_t> free_id_map;
	free_id_map[1] = 0;

	auto stmt2_obj = map_free_ids(free_id_map, stmt1_obj);

	// not identical nor equivalent
	// (try calling both ways round)
	BOOST_TEST(!semantics::identical(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(!semantics::identical(stmt2_obj,
		stmt1_obj));
	BOOST_TEST(!semantics::equivalent(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(!semantics::equivalent(stmt2_obj,
		stmt1_obj));
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

	BOOST_TEST(semantics::equivalent(stmt1, stmt2));
}


BOOST_DATA_TEST_CASE(test_get_successors,
	boost::unit_test::data::make({
		"*(x, x) = *( i(x), i(i(x)) )",
		"x = *(e, *(x, e))",
		"i(x) = e",
		"*(i(x), e) = x",
		"x = *(x, i(x))",
		"e = *(e, i(e))",
		"e = i(e)" }) ^
	boost::unit_test::data::make({
		"*(x, i(x)) = e",
		"*(x, *(y, z)) = *(*(x, y), z)",
		"x = x",
		"*(x, e) = x",
		"x = *(x, e)",
		"x = *(e, x)",
		"x = *(e, x)" }) ^
	boost::unit_test::data::make({
		"*(x, x) = e",
		"x = *(*(e, x), e)",
		"e = i(x)",
		"i(x) = x",
		"x = *(x, *(i(x), e))",
		"e = i(e)",
		"e = *(e, i(e))" }),
	subst_candidate, subst_rule, a_subst_result)
{
	// the statement to try substituting:
	s << subst_candidate << "\n";
	// the substitution rule(s):
	s << subst_rule << "\n";
	// a particular result of the substitution
	// (not necessarily all possibilities will be this,
	// but the test will pass iff at least one of the
	// substitution results is equivalent to this).
	s << a_subst_result;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto initial_stmt = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto rule_stmt = dynamic_cast<const Statement&>(p_stmts->at(1));
	auto target_stmt = dynamic_cast<const Statement&>(p_stmts->at(2));

	auto responses = semantics::get_successors(initial_stmt,
		{ rule_stmt });

	BOOST_TEST(responses.size() != 0);

	BOOST_TEST(std::any_of(responses.begin(), responses.end(),
		boost::bind(&semantics::equivalent,
			boost::ref(target_stmt), _1)));
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
	
	BOOST_TEST(semantics::implies(premise, concl));
}


BOOST_AUTO_TEST_SUITE_END();  // SemanticsTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


Statement map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	Statement stmt)
{
	std::map<size_t, SyntaxNodePtr> new_free_map;

	// change it so we map to syntax nodes
	for (auto pair : free_id_map)
	{
		new_free_map[pair.first] =
			FreeSyntaxNode::construct(pair.second);
	}

	// make the map total
	for (auto id : stmt.free_var_ids())
	{
		if (new_free_map.find(id) == new_free_map.end())
		{
			new_free_map[id] =
				FreeSyntaxNode::construct(id);
		}
	}

	// utilise the stmt's helper function for this
	return stmt.map_free_vars(new_free_map);
}


