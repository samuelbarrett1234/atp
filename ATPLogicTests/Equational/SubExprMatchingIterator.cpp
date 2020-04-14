/**
\file

\author Samuel Barrett

\brief This suite tests `atp::logic::equational::SubExprMatchingIterator`
*/


#include <Internal/Equational/SubExprMatchingIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/ProofState.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::logic::equational::SubExprMatchingIterator;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::ProofState;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(SubExprMatchingIteratorTests,
	StandardTestFixture);


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
		"*(x, x) = e",
		"x = *(*(e, x), e)",
		"e = i(x)",
		"i(x) = x",
		"x = *(x, *(i(x), e))",
		"e = i(e)",
		"e = *(e, i(e))" }),
	subst_candidate, a_subst_result)
{
	s << subst_candidate << "\n" << a_subst_result;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto a_result = dynamic_cast<const Statement&>(p_stmts->at(1));

	// need to offset forefront free var IDs so they don't clash,
	// as per the precondition of this iterator's constructors
	forefront = forefront.increment_free_var_ids(
		ker.get_rule_free_id_bound());

	auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
		forefront,  // doesn't matter what we put here
		forefront);

	std::vector<ProofStatePtr> results;
	while (p_iter->valid())
		results.emplace_back(p_iter->get());

	// at least one of the (potentially many) results should be
	// equivalent to `a_result`
	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&a_result](ProofStatePtr p)
		{
			return a_result.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}


BOOST_DATA_TEST_CASE(test_NOT_successors,
	boost::unit_test::data::make({
		"e = e",
		"x = e" }) ^
	boost::unit_test::data::make({
		"e = *(x, i(y))",
		"x = *(e, i(x))" }),
	subst_candidate, not_a_subst_result)
{
	s << subst_candidate << "\n" << not_a_subst_result;

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto not_a_result = dynamic_cast<const Statement&>(p_stmts->at(1));

	// need to offset forefront free var IDs so they don't clash,
	// as per the precondition of this iterator's constructors
	forefront = forefront.increment_free_var_ids(
		ker.get_rule_free_id_bound());

	auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
		forefront,  // doesn't matter what we put here
		forefront);

	std::vector<ProofStatePtr> results;
	while (p_iter->valid())
		results.emplace_back(p_iter->get());

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


