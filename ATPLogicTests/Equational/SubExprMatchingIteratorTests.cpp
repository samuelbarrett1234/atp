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
	StandardTestFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/RuleMatchingIteratorTests"));


BOOST_DATA_TEST_CASE(test_get_successors,
	boost::unit_test::data::make({
		"*(x, x) = *( i(x), i(i(x)) )",
		"x = *(e, *(x, e))",
		"*(i(x), e) = x",
		"x = *(x, i(x))",
		"e = *(e, i(e))",
		"e = i(e)" }) ^
	boost::unit_test::data::make({
		"*(x, x) = e",
		"x = *(*(e, x), e)",
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
		ker.get_rule_free_id_bound() + 1);

	ProofState pstate(ctx, ker, forefront);

	auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
		pstate, forefront);

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
		ker.get_rule_free_id_bound() + 1);

	ProofState pstate(ctx, ker, forefront);

	auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
		pstate, forefront);

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


BOOST_AUTO_TEST_CASE(test_subtly_different_statements_share_no_successors)
{
	// these two statements, while their differences are subtle,
	// should not share any successor statements (under equivalence)
	s << "*(x, y) = *(y, x)\n";
	s << "*(x, y) = *(x, y)";

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	const auto& _stmt1 = dynamic_cast<const Statement&>(
		p_stmts->at(0));
	const auto& _stmt2 = dynamic_cast<const Statement&>(
		p_stmts->at(1));

	// need to offset statement free var IDs so they don't clash,
	// as per the precondition of this iterator's constructors
	auto stmt1 = _stmt1.increment_free_var_ids(
		ker.get_rule_free_id_bound() + 1);
	auto stmt2 = _stmt2.increment_free_var_ids(
		ker.get_rule_free_id_bound() + 1);

	ProofState pstate1(ctx, ker, stmt1), pstate2(ctx, ker, stmt2);

	std::vector<ProofStatePtr> succs1, succs2;

	// get successors of stmt1
	{
		auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
			pstate1, stmt1);

		const size_t results_limit = 1000;  // shouldn't be more results than this
		for (size_t i = 0; i < results_limit && p_iter->valid();
			++i)
		{
			succs1.emplace_back(p_iter->get());
			p_iter->advance();
		}
	}

	// get successors of stmt2
	{
		auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
			pstate2, stmt2);

		const size_t results_limit = 1000;  // shouldn't be more results than this
		for (size_t i = 0; i < results_limit && p_iter->valid();
			++i)
		{
			succs2.emplace_back(p_iter->get());
			p_iter->advance();
		}
	}

	// no pair of statements in succs1 and succs2 should be
	// equivalent
	// i.e. for all statements in `succs1` there should not exist a
	// statement in `succs2` such that the two are equivalent
	for (auto _succ1 : succs1)
	{
		auto succ1 = dynamic_cast<ProofState*>(
			_succ1.get())->forefront();

		for (auto _succ2 : succs2)
		{
			auto succ2 = dynamic_cast<ProofState*>(
				_succ2.get())->forefront();

			BOOST_TEST(succ1.to_str() != succ2.to_str());
			BOOST_TEST(!succ1.equivalent(succ2));
		}
	}
}


struct NoFixt {};
BOOST_FIXTURE_TEST_CASE(test_no_resulting_matchings,
	NoFixt)
{
	// test that this iterator behaves itself when some
	// subexpressions have no matches with any of the
	// rules

	// of course we have to not use group theory for this test
	// because in group theory every expression has a match,
	// because you can aways multiply by the identity

	std::stringstream ctx_in;
	ctx_in << "{}";  // empty

	atp::logic::equational::Language lang;
	atp::logic::ModelContextPtr p_ctx = lang.try_create_context(
		ctx_in);
	const atp::logic::equational::ModelContext& ctx = dynamic_cast<const
		atp::logic::equational::ModelContext&>(*p_ctx);
	atp::logic::KnowledgeKernelPtr p_ker = lang.try_create_kernel(
		ctx);
	atp::logic::equational::KnowledgeKernel& ker = dynamic_cast<
		atp::logic::equational::KnowledgeKernel&>(*p_ker);

	std::stringstream s;

	s << "x = y";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto forefront = dynamic_cast<const Statement&>(p_stmts->at(0));

	forefront = forefront.increment_free_var_ids(1);
	ProofState pstate(ctx, ker, forefront);

	auto p_iter = SubExprMatchingIterator::construct(ctx, ker,
		pstate, forefront);

	BOOST_TEST(!p_iter->valid());
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


