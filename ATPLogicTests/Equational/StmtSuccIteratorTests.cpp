/**

\file

\author Samuel Barrett

\brief This suite tests the IProofState implementation for
	equational logic.

*/


#include <vector>
#include <Internal/Equational/StmtSuccIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::equational::StmtSuccIterator;
using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::StmtFormat;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StmtSuccIterTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ProofStateTests"));


// given several <statement, successor> pairs, test that the
// successor appears in the list of successors returned by
// the iterator
BOOST_DATA_TEST_CASE(test_is_a_succ,
	boost::unit_test::data::make({
		// starting statements
		"x = i(x)",
		"x = *(x, i(x))",
		"*(*(x, y), i(*(x, y))) = *(*(x, y), *(i(y), i(x)))",
		"*(*(x, y), i(*(x, y))) = *(x, i(x))",
		}) ^
	boost::unit_test::data::make({
		// one of the corresponding successor statements
		"x = *(e, i(x))",
		"x = e",
		"*(*(x, y), i(*(x, y))) = *(x, *(y, *(i(y), i(x))))",
		"*(*(x, y), i(*(x, y))) = *(x, *(e, i(x)))",
	}),
	stmt, succ_stmt)
{
	s << stmt << "\n" << succ_stmt;

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_arr != nullptr);
	BOOST_REQUIRE(p_arr->size() == 2);

	auto succ_iter = ker.begin_succession_of(p_arr->at(0));
	auto target_succ = dynamic_cast<const Statement&>(
		p_arr->at(1));

	// enumerate the successor proofs
	std::vector<Statement> succs;
	while (succ_iter->valid())
	{
		succs.emplace_back(static_cast<
			const Statement&>(succ_iter->get()));
		succ_iter->advance();
	}

	BOOST_TEST(std::any_of(succs.begin(),
		succs.end(),
		[&target_succ](const Statement& stmt)
		{
			return target_succ.equivalent(stmt);
		}));
}


// given several <statement, successor> pairs, test that the
// successor appears as a DEPTH TWO successor (which is reached
// via a call to `dive()`)
BOOST_DATA_TEST_CASE(test_dive_succ,
	boost::unit_test::data::make({
		// starting statements
		"x = i(x)",
		"x = x",
		"*(*(x, y), i(*(x, y))) = *(*(x, y), *(i(y), i(x)))",
		"*(*(x, y), i(*(x, y))) = *(x, i(x))",
		}) ^
	boost::unit_test::data::make({
		// one of the corresponding successor statements
		// of depth TWO
		"x = *(*(e, i(x)), e)",
		"x = x",
		"*(*(x, y), i(*(x, y))) = *(x, *(*(y, i(y)), i(x)))",
		"*(*(x, y), i(*(x, y))) = *(*(x, e), *(e, i(x)))",
		}),
	stmt, succ_stmt)
{
	s << stmt << "\n" << succ_stmt;

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_arr != nullptr);
	BOOST_REQUIRE(p_arr->size() == 2);

	auto succ_iter = ker.begin_succession_of(p_arr->at(0));
	auto target_succ = dynamic_cast<const Statement&>(
		p_arr->at(1));

	// enumerate the successor proofs
	std::vector<Statement> succs;
	while (succ_iter->valid())
	{
		auto next = succ_iter->dive();
		while (next->valid())
		{
			succs.emplace_back(static_cast<
				const Statement&>(next->get()));
			next->advance();
		}
		succ_iter->advance();
	}

	BOOST_TEST(std::any_of(succs.begin(),
		succs.end(),
		[&target_succ](const Statement& stmt)
		{
			return target_succ.equivalent(stmt);
		}));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


