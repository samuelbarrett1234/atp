/**

\file

\author

\brief This suite tests `atp::logic::equational::Expression`

\note The easiest way to obtain a statement object is to first parse
	a statement, and then get the expressions on either side of the
	equals sign. Thus you will see a lot in this suite, the loading
	of statements and then ignoring the statement object in favour
	for .lhs() and .rhs()

*/


#include <Internal/Equational/Expression.h>
#include <Internal/Equational/Statement.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::equational::Expression;
using atp::logic::equational::Statement;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ExpressionTests,
	StandardTestFixture,
	*boost::unit_test_framework::depends_on(
	"EquationalTests/ExprTreeFlyweightTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests"));


BOOST_AUTO_TEST_CASE(test_try_build_map_positive)
{
	// here we will extract the two sides of this equation separately
	// we will test to see if there exists a substitution for the LHS
	// which makes the RHS.
	s << "i(*(x, y)) = i(*(i(z), i(x)))";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	std::map<size_t, Expression> mapping;
	BOOST_TEST(stmt.lhs().try_match(stmt.rhs(), &mapping));
	BOOST_TEST(mapping.size() == 2);

	// try it again but with no output mapping (this is desirable
	// behaviour that has been designed for)
	BOOST_TEST(stmt.lhs().try_match(stmt.rhs(), nullptr));
}


BOOST_AUTO_TEST_CASE(test_try_build_map_negative)
{
	// here we will extract the two sides of this equation separately
	// we will test to see if there does not exist a substitution
	// for the LHS which makes the RHS.
	s << "i(*(e, y)) = i(*(i(z), i(x)))";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	std::map<size_t, Expression> mapping;
	BOOST_TEST(!stmt.lhs().try_match(stmt.rhs(), &mapping));
	BOOST_TEST(mapping.empty());

	// try it again but with no output mapping (this is desirable
	// behaviour that has been designed for)
	BOOST_TEST(!stmt.lhs().try_match(stmt.rhs(), nullptr));
}


BOOST_AUTO_TEST_CASE(test_try_build_map_empty)
{
	// check the case where there exists a free variable mapping,
	// but where that mapping is empty

	// here we will extract the two sides of this equation separately
	// we will test to see if there exists a substitution for the LHS
	// which makes the RHS.
	s << "e = e";

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto stmt = dynamic_cast<const Statement&>(p_stmts->at(0));

	std::map<size_t, Expression> mapping;
	BOOST_TEST(stmt.lhs().try_match(stmt.rhs(), &mapping));
	BOOST_TEST(mapping.empty());
}


BOOST_DATA_TEST_CASE(replace_tests,
	boost::unit_test::data::make({
		// expressions to start with
		"x", "i(x)", "i(x)", "*(*(x, x), x)" })
		^ boost::unit_test::data::make({
		// substitution
		"i(x)", "e", "e", "e" })
		^ boost::unit_test::data::make({
		// pre-order traversal index of the substitution
		// location
		0, 0, 1, 2 })
		^ boost::unit_test::data::make({
		// result
		"i(x)", "e", "i(e)", "*(*(e, x), x)" }), original_expr_lhs,
	sub_expr_lhs, sub_idx, result_expr_lhs)
{
	s << original_expr_lhs << " = x\n";  // arbitrary RHS
	s << sub_expr_lhs << " = x\n";  // arbitrary RHS
	s << result_expr_lhs << " = x\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto original = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();
	auto sub = dynamic_cast<const Statement&>(p_stmts->at(1)).lhs();
	auto correct_result = dynamic_cast<const Statement&>(p_stmts->at(2)).lhs();

	auto iter = original.begin();
	std::advance(iter, sub_idx);
	auto result = original.replace(iter, sub);

	BOOST_TEST(correct_result.equivalent(result));
}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


