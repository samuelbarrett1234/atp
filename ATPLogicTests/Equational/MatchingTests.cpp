/*

MatchingTests.cpp

This suite tests the functions that are found in the Matching.h/
Matching.cpp files. Those files contain an array of helper functions
and algorithms for dealing with syntax trees.

*/


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


struct MatchingTestsFixture
{
	MatchingTestsFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
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


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


