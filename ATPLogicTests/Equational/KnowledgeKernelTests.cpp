/**

\file

\author Samuel Barrett

\brief This suite tests the equational logic IKnowledgeKernel
	implementation.

*/


#include <Internal/Equational/KnowledgeKernel.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(KnowledgeKernelTests,
	StandardTestFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementArrayTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ModelContextTests"));


BOOST_AUTO_TEST_CASE(trivial_by_reflexivity)
{
	s << "i(x) = i(x)";
	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	BOOST_TEST(ker.is_trivial(stmts->at(0)));
}


BOOST_AUTO_TEST_CASE(trivial_by_theorem_equivalence)
{
	s << "e = *(y, i(y))";
	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	BOOST_TEST(ker.is_trivial(stmts->at(0)));
}


BOOST_AUTO_TEST_CASE(trivial_by_implied_by_axiom)
{
	s << "e = *(*(x, y), i(*(x, y)))";
	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	BOOST_TEST(ker.is_trivial(stmts->at(0)));
}


BOOST_AUTO_TEST_CASE(trivial_implied_by_axiom_but_transposed)
{
	s << "*(*(x, y), i(*(x, y))) = e";
	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	BOOST_TEST(ker.is_trivial(stmts->at(0)));
}


BOOST_DATA_TEST_CASE(test_not_trivial,
	boost::unit_test::data::make({
		"e = i(e)", "*(x, y) = *(y, x)",
		"*(i(x), y) = e",
		"e = *(e, i(x))", "*(*(x, y), e) = *(y, x)",
		"e = *(*(x, e), i(*(y, e)))" }),
	stmt_txt)
{
	s << stmt_txt;
	auto stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	BOOST_TEST(!ker.is_trivial(stmts->at(0)));
}


BOOST_AUTO_TEST_CASE(try_add_theorem)
{
	s << "e = i(e)";
	auto thms = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	ker.add_theorems(thms);
	BOOST_TEST(ker.is_trivial(thms->at(0)));
}


BOOST_AUTO_TEST_CASE(try_remove_theorem)
{
	s << "e = i(e)";
	auto thms = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	const auto ref = ker.add_theorems(thms);
	ker.remove_theorems(ref);
	BOOST_TEST(!ker.is_trivial(thms->at(0)));
}


BOOST_AUTO_TEST_CASE(test_axiom_matchings)
{
	// we have set the kernel to work within the context of group
	// theory - and of course, group theory has a non-zero number
	// of axioms!
	BOOST_TEST(ker.num_matching_rules() > 0);
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


