/*

MatchingTests.cpp

This suite tests the functions that are found in the Matching.h/
Matching.cpp files. Those files contain an array of helper functions
and algorithms for dealing with syntax trees.

*/


#include <boost/bind.hpp>
#include <Internal/Equational/Matching.h>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxTreeFold.h>
#include "../Test.h"
#include "SyntaxNodeToStr.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;
using atp::logic::equational::fold_syntax_tree;
namespace syntax_matching = atp::logic::equational::syntax_matching;


// apply the following map to free variable IDs (does not attempt to
// rebuild them afterwards) (returns a new tree) (IDs not present
// in the map are left unchanged - so for example, were the map to
// be empty, this would be the identity function.)
static SyntaxNodePtr map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	SyntaxNodePtr p_tree);


struct MatchingTestsFixture
{
	MatchingTestsFixture()
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


// here is the testing data for try_match
// note how every statement is <something>=x0
// this is because try_match accepts expressions, not Eq nodes, but
// the parser can only read statements with equality in them, so what
// we will do is: just examine the LHS of the equation.
// `match_patterns` is the list of statements for which we will
// search for a substitution.
// `match_trials` is the corresponding list of statements which the
// pattern will try to achieve (using a substitution).
// `match_subs` is another corresponding list of substitutions to
// be made for x0 in the pattern to achieve the trial.
// Since the elements in `match_subs` have arity 1, it means we can
// only deal with one substitution at a time. We will test higher
// arities in another place.
static std::string match_patterns[] =
{
	"x0 = x0",
	"*(x0, e) = x0",
	"*(x0, i(x0)) = x0"
};
static std::string match_trials[] =
{
	"*(x0, x1) = x0",
	"*(i(x0), e) = x0",
	"*(i(x0), i(i(x0))) = x0"
};
static std::string match_subs[] =
{
	"*(x0, x1)",
	"i(x0)",
	"i(x0)"
};
static size_t num_free_vars_in_subs[] =
{
	2, 1, 1
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(MatchingTests, MatchingTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFreeVarTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests"));


BOOST_DATA_TEST_CASE(test_try_match_gets_correct_substitutions,
	boost::unit_test::data::make(match_patterns)
	^ boost::unit_test::data::make(match_trials)
	^ boost::unit_test::data::make(match_subs)
	^ boost::unit_test::data::make(num_free_vars_in_subs),
	pattern, trial, sub, num_free_vars_in_sub)
{
	s << pattern;
	auto pattern_result = parse_statements(s);

	BOOST_REQUIRE(pattern_result.has_value());
	BOOST_REQUIRE(pattern_result.get().size() == 1);

	auto pattern_parse_tree = pattern_result.get().front();
	auto pattern_syntax_tree = ptree_to_stree(
		pattern_parse_tree, ker);

	s = std::stringstream();
	s << std::noskipws;

	s << trial;
	auto trial_result = parse_statements(s);

	BOOST_REQUIRE(trial_result.has_value());
	BOOST_REQUIRE(trial_result.get().size() == 1);

	auto trial_parse_tree = trial_result.get().front();
	auto trial_syntax_tree = ptree_to_stree(
		trial_parse_tree, ker);

	// should be no issues here
	BOOST_REQUIRE(pattern_syntax_tree != nullptr);
	BOOST_REQUIRE(trial_syntax_tree != nullptr);

	// now get LHS of both equations:

	auto p_pattern = dynamic_cast<EqSyntaxNode*>(
		pattern_syntax_tree.get())->left();
	auto p_trial = dynamic_cast<EqSyntaxNode*>(
		trial_syntax_tree.get())->left();

	auto match_result = syntax_matching::try_match(
		p_pattern, p_trial);

	BOOST_TEST(match_result.has_value());

	if (match_result.has_value())
	{
		// we only have one free variable in each statement, so
		// it would only be possible to make one substitution:
		BOOST_TEST(match_result.get().size() == 1);

		BOOST_TEST(exists_free_var_assignment(
			ker, match_result.get().begin()->second,
			sub, num_free_vars_in_sub));
	}
}


BOOST_AUTO_TEST_CASE(try_match_with_double_substitution)
{
	s << "*(*(x0, x1), x0) = x0";
	auto pattern_result = parse_statements(s);

	BOOST_REQUIRE(pattern_result.has_value());
	BOOST_REQUIRE(pattern_result.get().size() == 1);

	auto pattern_parse_tree = pattern_result.get().front();
	auto pattern_syntax_tree = ptree_to_stree(
		pattern_parse_tree, ker);

	s = std::stringstream();
	s << std::noskipws;

	s << "*(*(i(x0), *(x0, x1)), i(x0)) = x0";
	auto trial_result = parse_statements(s);

	BOOST_REQUIRE(trial_result.has_value());
	BOOST_REQUIRE(trial_result.get().size() == 1);

	auto trial_parse_tree = trial_result.get().front();
	auto trial_syntax_tree = ptree_to_stree(
		trial_parse_tree, ker);

	// should be no issues here
	BOOST_REQUIRE(pattern_syntax_tree != nullptr);
	BOOST_REQUIRE(trial_syntax_tree != nullptr);

	// now get LHS of both equations:

	auto p_pattern = dynamic_cast<EqSyntaxNode*>(
		pattern_syntax_tree.get())->left();
	auto p_trial = dynamic_cast<EqSyntaxNode*>(
		trial_syntax_tree.get())->left();

	auto match_result = syntax_matching::try_match(
		p_pattern, p_trial);

	BOOST_TEST(match_result.has_value());

	if (match_result.has_value())
	{
		BOOST_TEST(match_result.get().size() == 2);

		// get the substitutions as strings, as this is easiest
		// to check
		std::string returned_subs[] =
		{
			syntax_tree_to_str(ker,
				match_result.get().at(0)),
			syntax_tree_to_str(ker,
				match_result.get().at(1))
		};

		// note that x0 must be replaced with either
		// i(x0) or i(x1), and in the former case,
		// x1 must be replaced with *(x0, x1), and in
		// the latter case, x1 must be replaced with
		// *(x1, x0).

		BOOST_TEST(((returned_subs[0] == "i(x0)"
			&& returned_subs[1] == "*(x0, x1)") ||
			(returned_subs[0] == "i(x1)" &&
				returned_subs[1] == "*(x1, x0)")));
	}
}


BOOST_DATA_TEST_CASE(test_get_substitution,
	boost::unit_test::data::make({
		"x = x", "i(x) = e", "*(i(x), i(y)) = x"}) ^
	boost::unit_test::data::make({
		"i(x) = i(x)", "i(*(x, y)) = e", "*(i(y), i(y)) = y"}),
	start_stmt, target_stmt)
{
	s << start_stmt << std::endl << target_stmt;

	auto results = parse_statements(s);

	BOOST_REQUIRE(results.has_value());
	BOOST_REQUIRE(results.get().size() == 2);

	auto syntax_start_stmt = ptree_to_stree(results.get().front(),
		ker);
	auto syntax_target_stmt = ptree_to_stree(results.get().back(),
		ker);

	BOOST_REQUIRE(syntax_start_stmt != nullptr);
	BOOST_REQUIRE(syntax_target_stmt != nullptr);

	// extract the LHS from the start and the target

	auto p_pattern = dynamic_cast<EqSyntaxNode*>(
		syntax_start_stmt.get())->left();
	auto p_target = dynamic_cast<EqSyntaxNode*>(
		syntax_target_stmt.get())->left();

	auto sub = syntax_matching::try_match(p_pattern,
		p_target);

	BOOST_REQUIRE(sub.has_value());

	auto sub_result_stmt = syntax_matching::get_substitution(
		syntax_start_stmt, sub.get());

	BOOST_REQUIRE(sub_result_stmt != nullptr);

	// and finally, the important test:
	BOOST_TEST(syntax_matching::equivalent(*sub_result_stmt,
		*syntax_target_stmt));
}


BOOST_DATA_TEST_CASE(trivially_true_works_on_examples,
	boost::unit_test::data::make({
		// some statements to test on:
		"x=x", "x=y", "i(x)=x", "x=e", "e=e", "i(x)=i(x)",
		"*(x, y)=*(x, y)", "*(x, y)=*(y, x)" }) ^
	boost::unit_test::data::make({
		// whether or not those statements are trivially true
		true, false, false, false, true, true, true, false}),
	stmt, is_trivially_true)
{
	s << stmt;
	auto result = parse_statements(s);
	
	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree = ptree_to_stree(result.get().front(),
		ker);

	BOOST_REQUIRE(syntax_tree != nullptr);

	BOOST_TEST(syntax_matching::trivially_true(*syntax_tree)
		== is_trivially_true);
}


// firstly we will test the `identical` and `equivalent` functions
// in scenarios where they both should return the same value:
BOOST_DATA_TEST_CASE(test_identical_and_equivalent_work,
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

	BOOST_TEST(syntax_matching::identical(*syntax_tree_1,
		*syntax_tree_2) == is_equivalent_and_identical);
	BOOST_TEST(syntax_matching::equivalent(*syntax_tree_1,
		*syntax_tree_2) == is_equivalent_and_identical);
}


// now we will apply both functions to an example where the two
// statements are equivalent but not identical
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

	// change the free variable IDs so that they are no longer identical
	// (note that in some cases this may make the free IDs no longer
	// contiguous, but that shouldn't matter in this test.)
	std::map<size_t, size_t> free_id_map;
	free_id_map[0] = 1;  free_id_map[1] = 0;

	auto syntax_tree_2 = map_free_ids(free_id_map, syntax_tree_1);

	BOOST_REQUIRE(syntax_tree_2 != nullptr);

	// not identical, but equivalent
	BOOST_TEST(!syntax_matching::identical(*syntax_tree_1,
		*syntax_tree_2));
	BOOST_TEST(syntax_matching::equivalent(*syntax_tree_1,
		*syntax_tree_2));
}


// finally we will consider cases in which the statements are
// neither equivalent nor identical
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

	// map all used variables to ID 0!
	std::map<size_t, size_t> free_id_map;
	free_id_map[1] = 0;

	auto syntax_tree_2 = map_free_ids(free_id_map, syntax_tree_1);

	BOOST_REQUIRE(syntax_tree_2 != nullptr);

	// not identical, but equivalent
	BOOST_TEST(!syntax_matching::identical(*syntax_tree_1,
		*syntax_tree_2));
	BOOST_TEST(!syntax_matching::equivalent(*syntax_tree_1,
		*syntax_tree_2));
}


BOOST_DATA_TEST_CASE(test_num_free_vars,
	boost::unit_test::data::make({
		"x = x", "x = y", "*(x, *(y, z)) = *(*(x, y), z)",
		"i(x) = e", "e = e", "*(x, i(x)) = e" }) ^
	boost::unit_test::data::make({
		1, 2, 3, 1, 0, 1 }),
	stmt, num_free_vars)
{
	s << stmt;
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree = ptree_to_stree(result.get().front(),
		ker);

	BOOST_TEST(syntax_matching::num_free_vars(syntax_tree)
		== num_free_vars);
}


// now we will apply both functions to an example where the two
// statements are equivalent but not identical
BOOST_DATA_TEST_CASE(test_needs_free_var_id_rebuild_and_rebuilding,
	boost::unit_test::data::make({
		"x = y", "i(y) = x", "x = e", "*(x, y) = z" }), stmt)
{
	s << stmt;
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree_1 = ptree_to_stree(result.get().front(), ker);

	BOOST_REQUIRE(syntax_tree_1 != nullptr);

	// first test:
	BOOST_TEST(!syntax_matching::needs_free_var_id_rebuild(
		syntax_tree_1));

	// change the free variable IDs so that they need rebuilding
	// note that this map has been chosen carefully, so that:
	// - for each statement example, a rebuild is needed
	// - for each statement example, the IDs don't form a contiguous
	//   block
	// - for each example with <= 2 free variables, there is no free
	//   variable with ID 0.
	std::map<size_t, size_t> free_id_map;
	free_id_map[0] = 1;  free_id_map[1] = 5; free_id_map[2] = 0;

	auto syntax_tree_2 = map_free_ids(free_id_map, syntax_tree_1);

	BOOST_REQUIRE(syntax_tree_2 != nullptr);

	// second test:
	BOOST_TEST(syntax_matching::needs_free_var_id_rebuild(
		syntax_tree_2));

	// now try rebuilding! (modifies existing tree)
	syntax_matching::rebuild_free_var_ids(syntax_tree_2);

	// third test:
	BOOST_TEST(!syntax_matching::needs_free_var_id_rebuild(
		syntax_tree_2));
}


BOOST_AUTO_TEST_CASE(test_no_free_vars_means_doesnt_need_rebuild)
{
	// a statement with no free variables in it shouldn't need
	// rebuilding
	s << "e = e";
	auto result = parse_statements(s);

	BOOST_REQUIRE(result.has_value());
	BOOST_REQUIRE(result.get().size() == 1);

	auto syntax_tree = ptree_to_stree(result.get().front(), ker);

	BOOST_REQUIRE(syntax_tree != nullptr);

	BOOST_TEST(!syntax_matching::needs_free_var_id_rebuild(
		syntax_tree));
}


BOOST_AUTO_TEST_SUITE_END();  // MatchingTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


SyntaxNodePtr map_free_ids(
	const std::map<size_t, size_t>& free_id_map,
	SyntaxNodePtr p_tree)
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

	return fold_syntax_tree<SyntaxNodePtr>(eq_constructor,
		free_constructor, const_constructor, func_constructor,
		p_tree);
}


