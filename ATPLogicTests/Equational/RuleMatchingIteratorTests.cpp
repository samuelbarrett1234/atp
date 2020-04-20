/**
\file

\author Samuel Barrett

\brief This suite tests `atp::logic::equational::RuleMatchingIterator`
*/


#include <Internal/Equational/RuleMatchingIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/ProofState.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::logic::equational::RuleMatchingIterator;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::ProofState;


struct RuleMatchingIteratorTestsFixture :
	public StandardTestFixture
{
	std::vector<std::pair<size_t, SyntaxNodeType>> free_const_enum;

	RuleMatchingIteratorTestsFixture()
	{
		auto const_symb_ids = ctx.all_constant_symbol_ids();
		for (auto id : const_symb_ids)
		{
			free_const_enum.emplace_back(id,
				SyntaxNodeType::CONSTANT);
		}
	}
};


// settings for whether or not the iterator should randomise itself
static const bool randomised_settings[] =
{
	// repeat `true` a few times because each test will involve
	// randomness, so it's good to get a few repeats in
	false, true, true, true, true
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(RuleMatchingIteratorTests,
	RuleMatchingIteratorTestsFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/MatchResultsIteratorTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/MaybeRandomIndexTests"));


BOOST_DATA_TEST_CASE(test_subs,
	(boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"*(e, x) = x",
		"e = x",
		"e = x",
		"*(*(x, y), z) = x",
		"*(*(x, x), x) = x"
	}) ^
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		// (these are results obtainable in one step from the top
		// of the LHS of the corresponding statement above)
		"x = x",
		"*(x, i(x)) = x",
		"*(e, i(e)) = x",
		"*(x, *(y, z)) = x",
		"*(x, *(x, x)) = x" }))
	* boost::unit_test::data::make(randomised_settings),
	stmt, stmt_immediate_application, randomised)
{
	s << stmt << "\n" << stmt_immediate_application;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto a_result = dynamic_cast<const Statement&>(p_stmts->at(1));

	// register free variables
	auto free_ids = forefront.free_var_ids();
	for (auto id : free_ids)
		free_const_enum.emplace_back(id,
			SyntaxNodeType::FREE);

	// this indicates we match top of LHS
	auto sub_iter = forefront.begin();

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = RuleMatchingIterator::construct(ctx, ker,
		pstate, forefront, sub_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	std::vector<ProofStatePtr> results;
	const size_t results_limit = 1000;  // shouldn't be more results than this
	for (size_t i = 0; i < results_limit && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());

	// at least one of the (potentially many) results should be
	// equivalent to `a_result`
	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&a_result](ProofStatePtr p)
		{
			return a_result.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}


BOOST_DATA_TEST_CASE(test_NOT_subs,
	(boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		"e = x"
		}) ^
	boost::unit_test::data::make({
		// we will only look at the LHS of these statements
		// (these are results obtainable in one step from the top
		// of the LHS of the corresponding statement above)
		"*(e, i(x)) = x"
		}))
	* boost::unit_test::data::make(randomised_settings),
	stmt, stmt_not_immediate_application, randomised)
{
	s << stmt << "\n" << stmt_not_immediate_application;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto not_a_result = dynamic_cast<const Statement&>(p_stmts->at(1));

	// register free variables
	auto free_ids = forefront.free_var_ids();
	for (auto id : free_ids)
		free_const_enum.emplace_back(id,
			SyntaxNodeType::FREE);

	// this indicates we match top of LHS
	auto sub_iter = forefront.begin();

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = RuleMatchingIterator::construct(ctx, ker,
		pstate, forefront, sub_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	std::vector<ProofStatePtr> results;
	const size_t results_limit = 1000;  // shouldn't be more results than this
	for (size_t i = 0; i < results_limit && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());

	// at least one of the (potentially many) results should be
	// equivalent to `a_result`
	BOOST_TEST(std::none_of(results.begin(), results.end(),
		[&not_a_result](ProofStatePtr p)
		{
			return not_a_result.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


