/**
\file

\author Samuel Barrett

\brief Test suite for basic statement edit distance functionality.

*/


#include <Models/EditDistanceUtility.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::core::minimum_assignment;
using atp::core::edit_distance;
using atp::core::pairwise_edit_distance;
using atp::core::EditDistSubCosts;
namespace ublas = boost::numeric::ublas;
using atp::logic::StmtFormat;


struct EditDistanceTestsFixture :
	public LogicSetupFixture
{
	EditDistanceTestsFixture()
	{
		// setup costs

		sub_costs[std::make_pair(
			p_ctx->symbol_id("e"), p_ctx->symbol_id("e")
		)] = 0.0f;

		sub_costs[std::make_pair(
			p_ctx->symbol_id("i"), p_ctx->symbol_id("e")
		)] = 10.0f;

		sub_costs[std::make_pair(
			p_ctx->symbol_id("i"), p_ctx->symbol_id("i")
		)] = 0.0f;

		sub_costs[std::make_pair(
			p_ctx->symbol_id("*"), p_ctx->symbol_id("e")
		)] = 10.0f;

		sub_costs[std::make_pair(
			p_ctx->symbol_id("*"), p_ctx->symbol_id("i")
		)] = 10.0f;

		sub_costs[std::make_pair(
			p_ctx->symbol_id("*"), p_ctx->symbol_id("*")
		)] = 0.0f;
	}

	EditDistSubCosts sub_costs;
};


BOOST_FIXTURE_TEST_SUITE(EditDistanceTests,
	EditDistanceTestsFixture);


BOOST_AUTO_TEST_CASE(minimum_assignment_square_test)
{
	ublas::matrix<float> dists(3, 3);

	dists(0, 0) = 33.0f; dists(0, 1) = 1.0f; dists(0, 2) = 10.0f;
	dists(1, 0) = 1.0f; dists(1, 1) = 2.0f; dists(1, 2) = 5.0f;
	dists(2, 0) = 10.0f; dists(2, 1) = 5.0f; dists(2, 2) = 6.0f;

	BOOST_TEST(minimum_assignment(dists) == 8.0f);
}


BOOST_AUTO_TEST_CASE(minimum_assignment_not_square_test)
{
	ublas::matrix<float> dists(3, 2);

	dists(0, 0) = 33.0f; dists(0, 1) = 1.0f;
	dists(1, 0) = 1.0f; dists(1, 1) = 2.0f;
	dists(2, 0) = 10.0f; dists(2, 1) = 5.0f;

	BOOST_TEST(minimum_assignment(dists) == 2.0f);
}


BOOST_DATA_TEST_CASE(test_edit_dist,
	boost::unit_test::data::make({
		"x0 = x0",
		"e = e",
		"e = x0",
		"e = e",
		"i(x0) = e",
		"e = e",
		"*(x0, x1) = i(e)",
		"*(x0, i(i(e))) = i(e)"
		}) ^
	boost::unit_test::data::make({
		"x0 = x0",
		"e = e",
		"e = e",
		"e = x0",
		"e = e",
		"i(x0) = e",
		"*(x0, i(e)) = x0",
		"*(i(e), i(e)) = x0"
		}) ^
	boost::unit_test::data::make({
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		10.0f,
		10.0f,
		2.0f,
		1.0f + 1.0f + 10.0f
		}), stmt1_str, stmt2_str, target_dist)
{
	s << stmt1_str << "\n" << stmt2_str;

	auto p_stmts = p_lang->deserialise_stmts(s, StmtFormat::TEXT,
		*p_ctx);

	float dist = edit_distance(p_stmts->at(0), p_stmts->at(1),
		sub_costs);

	BOOST_TEST(dist == target_dist);
}


BOOST_DATA_TEST_CASE(test_pairwise_edit_dist,
	boost::unit_test::data::make({
		"x0 = x0",
		"e = e",
		"e = x0",
		"e = e",
		"i(x0) = e",
		"e = e",
		"*(x0, x1) = i(e)",
		"*(x0, i(i(e))) = i(e)"
		}) ^
	boost::unit_test::data::make({
		"x0 = x0",
		"e = e",
		"e = e",
		"e = x0",
		"e = e",
		"i(x0) = e",
		"*(x0, i(e)) = x0",
		"*(i(e), i(e)) = x0"
		}) ^
	boost::unit_test::data::make({
		0.0f,
		0.0f,
		1.0f,
		1.0f,
		10.0f,
		10.0f,
		2.0f,
		1.0f + 1.0f + 10.0f
		}), stmt1_str, stmt2_str, target_dist)
{
	s << stmt1_str << "\n" << stmt2_str;

	auto p_stmts = p_lang->deserialise_stmts(s, StmtFormat::TEXT,
		*p_ctx);

	const auto dists = pairwise_edit_distance(*p_stmts, *p_stmts,
		sub_costs);

	BOOST_REQUIRE(dists.size() == 2);
	BOOST_REQUIRE(dists.front().size() == 2);
	BOOST_REQUIRE(dists.back().size() == 2);
	BOOST_TEST(dists[0][0] == 0.0f);
	BOOST_TEST(dists[1][1] == 0.0f);
	BOOST_TEST(dists[0][1] == target_dist);
	BOOST_TEST(dists[1][0] == target_dist);
}


BOOST_AUTO_TEST_SUITE_END();


