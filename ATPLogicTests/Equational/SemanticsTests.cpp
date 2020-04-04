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
		symb_id_to_arity[ker.symbol_id("e")] = 0;
		ker.define_symbol("i", 1);
		symb_id_to_arity[ker.symbol_id("i")] = 1;
		ker.define_symbol("*", 2);
		symb_id_to_arity[ker.symbol_id("*")] = 2;
		s << std::noskipws;
	}

	std::stringstream s;
	KnowledgeKernel ker;
	std::map<size_t, size_t> symb_id_to_arity;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SemanticsTests, SemanticsTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests"));


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
		semantics::transpose(stmt2)));
	BOOST_TEST(semantics::identical(stmt2,
		semantics::transpose(stmt1)));
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
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree_1 = ptree_to_stree(result.get().front(), ker);

	BOOST_REQUIRE(syntax_tree_1 != nullptr);

	auto stmt1_obj = Statement(ker, syntax_tree_1);

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
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 2);

	auto stree1 = ptree_to_stree(result.get().front(), ker);
	auto stree2 = ptree_to_stree(result.get().back(), ker);

	auto stmt1 = Statement(ker, stree1);
	auto stmt2 = Statement(ker, stree2);

	BOOST_TEST(semantics::equivalent(stmt1, stmt2));
}


BOOST_AUTO_TEST_CASE(test_get_substitutions)
{
	// the statement to try substituting:
	s << "*(x, x) = *( i(x), i(i(x)) )\n";
	// the substitution rule(s):
	s << "*(x, i(x)) = e\n";
	// the result of the substitution:
	s << "*(x, x) = e";

	auto results = parse_statements(s);
	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 3);

	auto parse_result_iter = results->begin();
	auto initial_stmt = Statement(ker, ptree_to_stree(
		*parse_result_iter, ker));
	++parse_result_iter;
	auto rule_stmt = Statement(ker, ptree_to_stree(
		*parse_result_iter, ker));
	++parse_result_iter;
	auto sub_result_stmt = Statement(ker, ptree_to_stree(
		*parse_result_iter, ker));

	auto responses = semantics::get_substitutions(initial_stmt,
		{ rule_stmt });

	// there was exactly one way to apply that rule:
	BOOST_TEST(responses.size() == 1);

	for (auto stmt : responses)
	{
		BOOST_TEST(semantics::equivalent(sub_result_stmt,
			stmt));
	}
}


BOOST_AUTO_TEST_CASE(test_replace_free_with_def)
{
	s << "x0 = i(x0) \n";
	s << "e = i(e)\n";  // this is one of the possible answers
	s << "i(x0) = i(i(x0))\n";  // another possible answer
	s << "*(x0, x1) = i(*(x0, x1))";  // the final possible answer
	// warning: with the last one, we can of course swap the two
	// free variables around without issue.

	auto results = parse_statements(s);
	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 4);

	auto iter = results->begin();
	auto initial_stmt = Statement(ker, ptree_to_stree(
		*iter, ker));
	iter++;
	std::unique_ptr<Statement> answers[3];
	for (size_t i = 0; i < 3; i++)
	{
		answers[i] = std::make_unique<Statement>(ker,
			ptree_to_stree(*iter, ker));
		iter++;
	}

	auto responses = semantics::replace_free_with_def(initial_stmt,
		symb_id_to_arity);

	BOOST_REQUIRE(responses.size() == 3);

	// which_is_which[i] represents: which (unique) answer was
	// equivalent to responses[i]?
	size_t which_is_which[3];
	for (size_t i = 0; i < 3; i++)
	{
		bool set = false;
		for (size_t j = 0; j < 3; j++)
		{
			BOOST_TEST(!set);
			if (semantics::equivalent(responses.my_at(i),
				*answers[j]))
			{
				set = true;
				which_is_which[i] = j;
				break;
			}
		}
		BOOST_TEST(set);
	}

	// now check that no two statements were equivalent to
	// the same answer:
	BOOST_TEST(which_is_which[0] != which_is_which[1]);
	BOOST_TEST(which_is_which[1] != which_is_which[2]);
	BOOST_TEST(which_is_which[0] != which_is_which[2]);
}


BOOST_AUTO_TEST_CASE(test_replace_free_with_free)
{
	// initial:
	s << "*(x0, i(x1)) = *(e, i(x1)) \n";

	// all answers equivalent to this:
	s << "*(x0, i(x0)) = *(e, i(x0))";

	auto results = parse_statements(s);
	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto initial_stmt = Statement(ker, ptree_to_stree(
		results->front(), ker));
	auto answer_stmt = Statement(ker, ptree_to_stree(
		results->back(), ker));

	auto responses = semantics::replace_free_with_free(initial_stmt);

	// there are two free variables, so the only unique way to do
	// a substitution is by replacing one of the variables by the
	// other, leaving only one variable left; of course, there are
	// to symmetric ways of doing this, thus by testing that size()
	// == 1, we are asking the implementation to avoid this symmetry:
	BOOST_TEST(responses.size() == 1);

	// all responses should be equivalent to the "answer statement"
	for (auto stmt : responses)
	{
		BOOST_TEST(semantics::equivalent(stmt, answer_stmt));
	}
}


BOOST_DATA_TEST_CASE(test_follows_from,
	boost::unit_test::data::make({
		// patterns
		"x0 = x0",
		"*(x0, e) = x0",
		"*(x0, i(x0)) = e",
		"e = *( x, i(x) )" })
		^ boost::unit_test::data::make({
		// targets
		"i(x0) = i(x0)",
		"*(i(x0), e) = i(x0)",
		"*(i(x0), i(i(x0))) = e",
		"e = *( i(x), i(i(x)) )" }),
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



BOOST_AUTO_TEST_SUITE_END();  // SemanticsTests
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


