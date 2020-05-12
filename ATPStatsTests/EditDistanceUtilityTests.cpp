/**
\file

\author Samuel Barrett

\brief Test suite for basic statement edit distance functionality.

*/


#include <ATPStatsEditDistance.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::stats::EditDistancePtr;
using atp::stats::create_edit_dist;
using atp::stats::EditDistSubCosts;
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

		p_ed = create_edit_dist(
			atp::logic::LangType::EQUATIONAL_LOGIC,
			sub_costs);
	}

	EditDistSubCosts sub_costs;
	EditDistancePtr p_ed;
};


BOOST_FIXTURE_TEST_SUITE(EditDistanceTests,
	EditDistanceTestsFixture,
	* boost::unit_test_framework::depends_on(
	"MinAssignmentTests"));


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

	float dist = p_ed->edit_distance(
		p_stmts->at(0), p_stmts->at(1));

	BOOST_TEST(dist == target_dist);
}


BOOST_AUTO_TEST_SUITE_END();


