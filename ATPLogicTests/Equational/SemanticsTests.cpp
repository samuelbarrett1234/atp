/*

SemanticsTests.cpp

This suite tests the functions that are found in the Matching.h/
Matching.cpp files. Those files contain an array of helper functions
and algorithms for dealing with syntax trees.

*/


#include <boost/bind.hpp>
#include <Internal/Equational/Semantics.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxNodes.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"


using atp::logic::equational::Statement;
using atp::logic::equational::KnowledgeKernel;
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


struct SemanticsTestsFixture
{
	SemanticsTestsFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
		s << std::noskipws;
	}

	std::stringstream s;
	KnowledgeKernel ker;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SemanticsTests, SemanticsTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests"));


BOOST_DATA_TEST_CASE(test_follows_from,
	boost::unit_test::data::make({
		// patterns
		"x0 = x0",
		"*(x0, e) = x0",
		"*(x0, i(x0)) = e" })
	^ boost::unit_test::data::make({
		// targets
		"i(x0) = i(x0)",
		"*(i(x0), e) = i(x0)",
		"*(i(x0), i(i(x0))) = e" }),
	pattern, target)
{
	s << pattern << "\n" << target;
	auto results = parse_statements(s);

	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto stree1 = ptree_to_stree(results.get().front(), ker);
	auto stree2 = ptree_to_stree(results.get().back(), ker);

	BOOST_REQUIRE(stree1 != nullptr);
	BOOST_REQUIRE(stree2 != nullptr);

	auto pattern_stmt = Statement(ker, stree1);
	auto target_stmt = Statement(ker, stree2);

	BOOST_TEST(semantics::follows_from(pattern_stmt, target_stmt));
}


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
	auto result = parse_statements(s);
	
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree = ptree_to_stree(result.get().front(),
		ker);

	BOOST_REQUIRE(syntax_tree != nullptr);

	auto stmt_obj = Statement(ker, syntax_tree);

	BOOST_TEST(semantics::true_by_reflexivity(stmt_obj)
		== is_symmetric);
}


// firstly we will test the `identical` and `equivalent` functions
// in scenarios where they both should return the same value:
BOOST_DATA_TEST_CASE(test_identical_and_equivalent,
	boost::unit_test::data::make({
	// first list of statements
	"x = x", "i(x) = x", "x = e", "e = e" }) ^
	boost::unit_test::data::make({
	// second list of statements
	"y = y", "i(y) = y", "e = e", "e = e" }) ^
	boost::unit_test::data::make({
	// whether or not those statements are equivalent
	// (and also identical - it makes no difference here) and
	// note that "x=x" and "y=y" are identical because, once
	// the statements are parsed, both have 0 as their free
	// variable IDs.
	true, true, false, true }),
	stmt1, stmt2, is_equivalent_and_identical)
{
	// parse them both at the same time
	s << stmt1 << std::endl << stmt2;
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 2);

	auto syntax_tree_1 = ptree_to_stree(result.get().front(),
		ker);
	auto syntax_tree_2 = ptree_to_stree(result.get().back(),
		ker);

	BOOST_REQUIRE(syntax_tree_1 != nullptr);
	BOOST_REQUIRE(syntax_tree_2 != nullptr);

	auto stmt1_obj = Statement(ker, syntax_tree_1);
	auto stmt2_obj = Statement(ker, syntax_tree_2);

	BOOST_TEST(semantics::identical(stmt1_obj,
		stmt2_obj) == is_equivalent_and_identical);
	BOOST_TEST(semantics::equivalent(stmt1_obj,
		stmt2_obj) == is_equivalent_and_identical);
}


BOOST_DATA_TEST_CASE(test_transpose,
	boost::unit_test::data::make({ "x0 = i(x0)",
		"i(x0) = *(x0, x1)" }) ^
	boost::unit_test::data::make({
		"i(x0) = x0", "*(x0, x1) = i(x0)" }),
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
		semantics::transpose(stmt2)));
	BOOST_TEST(semantics::identical(stmt2,
		semantics::transpose(stmt1)));
}


// now we will apply both functions to an example where the two
// statements are equivalent but not identical (we will obtain
// the second statement from the ones given in the list by
// remapping the free variable IDs.)
BOOST_DATA_TEST_CASE(test_equivalent_but_not_identical,
	boost::unit_test::data::make({
		"x = x", "i(x) = x", "x = e", "*(x, y) = x" }), stmt)
{
	s << stmt;
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree_1 = ptree_to_stree(result.get().front(), ker);

	BOOST_REQUIRE(syntax_tree_1 != nullptr);

	auto stmt1_obj = Statement(ker, syntax_tree_1);

	// change the free variable IDs so that they are no longer
	// identical
	std::map<size_t, size_t> free_id_map;
	free_id_map[0] = 1;  free_id_map[1] = 0;

	auto stmt2_obj = map_free_ids(free_id_map, stmt1_obj);

	// not identical, but equivalent
	BOOST_TEST(!semantics::identical(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(semantics::equivalent(stmt1_obj,
		stmt2_obj));
}


BOOST_AUTO_TEST_CASE(test_equivalent_invariant_to_reflection)
{
	// reflect the statement along the equals sign, should still
	// be equivalent but not identical
	s << "x = i(x) \n i(x) = x";
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 2);

	auto stree1 = ptree_to_stree(result.get().front(), ker);
	auto stree2 = ptree_to_stree(result.get().back(), ker);

	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);

	BOOST_TEST(semantics::equivalent(stmt1, stmt2));
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
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree_1 = ptree_to_stree(result.get().front(), ker);

	BOOST_REQUIRE(syntax_tree_1 != nullptr);

	auto stmt1_obj = Statement(ker, syntax_tree_1);

	// map all used variables to ID 0!
	std::map<size_t, size_t> free_id_map;
	free_id_map[1] = 0;

	auto stmt2_obj = map_free_ids(free_id_map, stmt1_obj);

	// not identical, but equivalent
	BOOST_TEST(!semantics::identical(stmt1_obj,
		stmt2_obj));
	BOOST_TEST(!semantics::equivalent(stmt1_obj,
		stmt2_obj));
}


BOOST_AUTO_TEST_SUITE_END();  // MatchingTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


Statement map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	Statement stmt)
{
	// this is just another fold, where the eq/const/func
	// constructors are just trivial (they construct said
	// objects) whereas the free constructor first checks
	// as to whether the given ID is present in the free_id_map
	// and if it is, it returns the corresponding mapped ID.
	// Otherwise it keeps the ID as-is.

	auto eq_constructor = boost::bind(
		&std::make_shared<EqSyntaxNode, SyntaxNodePtr, SyntaxNodePtr>,
		_1, _2);

	// (using function composition via nested boost::bind)
	auto free_constructor = boost::bind(
		&std::make_shared<FreeSyntaxNode, size_t>,
		boost::bind<size_t>([&free_id_map](size_t id) -> size_t
		{
			auto iter = free_id_map.find(id);
			if (iter != free_id_map.end())
				return iter->second;
			else
				return id;
		}, _1));

	auto const_constructor = boost::bind(
		&std::make_shared<ConstantSyntaxNode, size_t>, _1);

	auto func_constructor = boost::bind(
		&std::make_shared<FuncSyntaxNode, size_t,
		std::list<SyntaxNodePtr>::iterator,
		std::list<SyntaxNodePtr>::iterator>, _1, _2, _3);

	return Statement(stmt.kernel(), stmt.fold<SyntaxNodePtr>(eq_constructor,
		free_constructor, const_constructor, func_constructor));
}


