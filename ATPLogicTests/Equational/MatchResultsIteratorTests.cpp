/**
\file

\author Samuel Barrett

\brief This suite tests `atp::logic::equational::MatchResultsIterator`
*/


#include <Internal/Equational/MatchResultsIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/ProofState.h>
#include <Internal/FreeVarIdSet.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::logic::FreeVarIdSet;
using atp::logic::equational::MatchResultsIterator;
using atp::logic::equational::Expression;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::ProofState;


// settings for whether or not the iterator should randomise itself
static const bool randomised_settings[] =
{
	// repeat `true` a few times because each test will involve
	// randomness, so it's good to get a few repeats in
	false, true, true, true, true
};


struct MatchResultsIteratorTestsFixture :
	public StandardTestFixture
{
	std::vector<std::pair<size_t, SyntaxNodeType>> free_const_enum;

	MatchResultsIteratorTestsFixture()
	{
		auto const_symb_ids = ctx.all_constant_symbol_ids();
		for (auto id : const_symb_ids)
		{
			free_const_enum.emplace_back(id,
				SyntaxNodeType::CONSTANT);
		}
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(MatchResultsIteratorTests,
	MatchResultsIteratorTestsFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/FreeVarAssignmentIteratorTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/MaybeRandomIndexTests"));


BOOST_DATA_TEST_CASE(test_constructs_correct_substitution,
	boost::unit_test::data::make(randomised_settings),
	randomised)
{
	// forefront statement
	s << "*(i(x), i(i(x))) = x\n";
	// the LHS of this is a match result
	s << "e = e\n";
	// the result:
	s << "e = x\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto match_result = dynamic_cast<const Statement&>(p_stmts->at(1));
	auto final_result = dynamic_cast<const Statement&>(p_stmts->at(2));

	free_const_enum.emplace_back(0,
		SyntaxNodeType::FREE);  // register x

	auto sub_iter = forefront.begin();

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = MatchResultsIterator::construct(ctx, ker,
		pstate, forefront,
		{ std::make_pair(match_result.lhs(), FreeVarIdSet()) },
		sub_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	std::vector<ProofStatePtr> results;
	const size_t expected_num_results = 1;
	for (size_t i = 0; i < expected_num_results && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());

	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&final_result](ProofStatePtr p)
		{
			return final_result.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}


BOOST_DATA_TEST_CASE(test_many_possible_results,
	boost::unit_test::data::make(randomised_settings),
	randomised)
{
	// forefront statement
	s << "x = x\n";
	// the sides of this are the match results
	s << "*(x, y) = *(e, x)\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto match_results = dynamic_cast<const Statement&>(p_stmts->at(1));

	free_const_enum.emplace_back(0,
		SyntaxNodeType::FREE);  // register x

	std::vector<std::pair<Expression, FreeVarIdSet>>
		match_results_array = {
		std::make_pair(match_results.lhs(),
			FreeVarIdSet(std::vector<size_t>{ 1 })),
		std::make_pair(match_results.rhs(),
			FreeVarIdSet())
	};

	auto sub_iter = forefront.begin();

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = MatchResultsIterator::construct(ctx, ker,
		pstate, forefront,
		{
			std::make_pair(match_results.lhs(),
				FreeVarIdSet(std::vector<size_t>{ 1 })),
			std::make_pair(match_results.rhs(),
				FreeVarIdSet())
		},
		sub_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	std::vector<ProofStatePtr> results;
	const size_t expected_num_results = 3;
	for (size_t i = 0; i < expected_num_results && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());

	// the first match produces:
	// *(x, x) = x
	// *(x, e) = x
	// the second match produces:
	// *(e, x) = x
}


BOOST_DATA_TEST_CASE(test_it_passes_the_correct_remaining_free_ids,
	boost::unit_test::data::make(randomised_settings),
	randomised)
{
	// forefront statement
	s << "x = *(x, y)\n";
	// the LHS of this is a match result
	// (note that the x here DOES link up with the forefront stmt x,
	// but the y and the z do not).
	s << "*(x, *(y, z)) = e\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto match_result = dynamic_cast<const Statement&>(p_stmts->at(1));

	free_const_enum.emplace_back(0,
		SyntaxNodeType::FREE);  // register x
	free_const_enum.emplace_back(1,
		SyntaxNodeType::FREE);  // register y

	auto sub_iter = forefront.begin();

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = MatchResultsIterator::construct(ctx, ker,
		pstate, forefront,
		{ std::make_pair(match_result.lhs(),
			FreeVarIdSet(std::vector<size_t>{ 1, 2 })) },
		sub_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	// The results are of the form:
	// *(x, *(z, w)) = *(x, y)
	// Where x and y are fixed, but z and w can be assigned values
	// e, x, y
	// Hence there are 3^2 = 9 possible results
	std::vector<ProofStatePtr> results;
	const size_t expected_num_results = 9;
	for (size_t i = 0; i < expected_num_results && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());
}


BOOST_DATA_TEST_CASE(test_sub_at_inner_location,
	boost::unit_test::data::make(randomised_settings),
	randomised)
{
	// forefront statement
	s << "*(*(x, y), z) = e\n";
	// lhs of this happens to be a match result
	s << "i(x) = e\n";
	// result of substitution
	s << "*(*(i(x), y), z) = e\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto match_result = dynamic_cast<const Statement&>(p_stmts->at(1));
	auto final_result = dynamic_cast<const Statement&>(p_stmts->at(2));

	free_const_enum.emplace_back(0,
		SyntaxNodeType::FREE);  // register x
	free_const_enum.emplace_back(1,
		SyntaxNodeType::FREE);  // register y
	free_const_enum.emplace_back(2,
		SyntaxNodeType::FREE);  // register z

	auto sub_loc_iter = forefront.begin();
	std::advance(sub_loc_iter, 2);

	ProofState pstate(ctx, ker, forefront, false, randomised);

	auto p_iter = MatchResultsIterator::construct(ctx, ker,
		pstate, forefront,
		{ std::make_pair(match_result.lhs(), FreeVarIdSet()) },
		sub_loc_iter, free_const_enum, randomised);

	BOOST_TEST(p_iter->valid());

	std::vector<ProofStatePtr> results;
	const size_t expected_num_results = 1;
	for (size_t i = 0; i < expected_num_results && p_iter->valid();
		++i)
	{
		results.emplace_back(p_iter->get());
		p_iter->advance();
	}
	BOOST_TEST(!p_iter->valid());

	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&final_result](ProofStatePtr p)
		{
			return final_result.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


