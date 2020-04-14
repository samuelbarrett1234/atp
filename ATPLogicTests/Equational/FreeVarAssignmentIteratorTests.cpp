/**
\file

\author Samuel Barrett

\brief This suite tests `atp::logic::equational::FreeVarAssignmentIterator`
*/


#include <Internal/Equational/FreeVarAssignmentIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/ProofState.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::logic::equational::FreeVarAssignmentIterator;
using atp::logic::equational::Statement;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::ProofState;


struct FreeVarAssignmentIteratorTestsFixture :
	public StandardTestFixture
{
	std::vector<std::pair<size_t, SyntaxNodeType>> free_const_enum;
	std::vector<size_t> remaining_free;

	FreeVarAssignmentIteratorTestsFixture()
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
BOOST_FIXTURE_TEST_SUITE(FreeVarAssignmentIteratorTests,
	FreeVarAssignmentIteratorTestsFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests"));


BOOST_TEST_DECORATOR(* boost::unit_test_framework::timeout(1))
BOOST_AUTO_TEST_CASE(test_one_var)
{
	// test substitution when there is only one free variable this
	// iterator should be substituting (but with more than one in the
	// statement).
	s << "*(x, y) = *(y, x)\n";

	// the possible results:
	s << "*(e, y) = *(y, e)\n";
	s << "*(y, y) = *(y, y)\n";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));
	auto result1 = dynamic_cast<const Statement&>(p_stmts->at(1));
	auto result2 = dynamic_cast<const Statement&>(p_stmts->at(2));

	remaining_free = { 0 };  // only want to substitute one free var

	// enumerate the other free variable
	free_const_enum.emplace_back(1, SyntaxNodeType::FREE);

	BOOST_REQUIRE(free_const_enum.size() == 2);  //  e and y

	auto p_iter = FreeVarAssignmentIterator::construct(ctx, ker,
		stmt /* can pass anything for this as its irrelevant to us*/,
		stmt, free_const_enum, remaining_free.begin(),
		remaining_free.end());

	std::vector<ProofStatePtr> results;
	while (p_iter->valid())
		results.emplace_back(p_iter->get());

	BOOST_TEST(results.size() == 2);

	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&result1](ProofStatePtr p)
		{
			return result1.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));

	BOOST_TEST(std::any_of(results.begin(), results.end(),
		[&result2](ProofStatePtr p)
		{
			return result2.equivalent(
				dynamic_cast<ProofState*>(p.get())->forefront());
		}));
}



BOOST_TEST_DECORATOR(*boost::unit_test_framework::timeout(1))
BOOST_AUTO_TEST_CASE(test_many_vars)
{
	// check that we are doing a cartesian product on the possible
	// substitutions properly - if we substitute four out of five
	// free variables here, then there are 2^4 possibilities, because
	// each free variable that we are substituting can be replaced
	// either by the constant `e` or the remaining free variable.
	s << "*(x, *(y, x)) = *(*(t, u), v)";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	remaining_free = { 0, 1, 2, 3 };  // sub all but last one

	// enumerate the other free variable
	free_const_enum.emplace_back(4, SyntaxNodeType::FREE);

	BOOST_REQUIRE(free_const_enum.size() == 2);

	auto p_iter = FreeVarAssignmentIterator::construct(ctx, ker,
		stmt /* can pass anything for this as its irrelevant to us*/,
		stmt, free_const_enum, remaining_free.begin(),
		remaining_free.end());

	std::vector<ProofStatePtr> results;
	while (p_iter->valid())
		results.emplace_back(p_iter->get());

	// two possibilities for each free variable
	// (16 = 2^4)
	BOOST_TEST(results.size() == 16);

	// don't bother checking the values of the permutations, only
	// check the correct number
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


