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


BOOST_DATA_TEST_CASE(iterator_walking_test,
	boost::unit_test::data::make({
		"x0", "e", "i(x0)", "i(x0)", "*(*(x0, e), e)",
		"*(*(x0, e), e)" }) ^
	boost::unit_test::data::make({
		0, 0, 0, 1, 2, 4 }) ^
	boost::unit_test::data::make({
		"x0", "e", "i(x0)", "x0", "x0", "e" }),
	expr_str, advance_num, sub_expr_str)
{
	s << expr_str << " = x0\n";  // arbitrary RHS
	s << sub_expr_str << " = x0\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto original = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();
	auto sub = dynamic_cast<const Statement&>(p_stmts->at(1)).lhs();

	auto iter = original.begin();
	std::advance(iter, advance_num);

	BOOST_TEST(sub.equivalent(*iter));
	BOOST_TEST(iter->equivalent(sub));
}


BOOST_DATA_TEST_CASE(end_iterator_test,
	boost::unit_test::data::make({
		"x0", "e", "i(x0)", "*(*(x0, e), e)",
		"*(*(x0, e), i(e))" }) ^
	boost::unit_test::data::make({
		1, 1, 2, 5, 6 }),
	expr_str, advance_num)
{
	s << expr_str << " = x0\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto expr = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();

	auto iter = expr.begin();

	BOOST_TEST(iter.is_begin_iterator());

	std::advance(iter, advance_num);

	BOOST_TEST(iter.is_end_iterator());
	BOOST_TEST((iter == expr.end()));
}


BOOST_DATA_TEST_CASE(parent_position_iterator_test,
	boost::unit_test::data::make({
		// expression string
		"i(x0)", "*(*(x0, e), e)",
		"*(*(x0, e), i(e))", "*(*(x0, e), e)" }) ^
	boost::unit_test::data::make({
		// advance number
		1, 1, 5, 3 }) ^
	// can't really test .parent_func_idx() since we have no
	// guarantees how those indices will be assigned
	boost::unit_test::data::make({
		// parent function argument
		0, 0, 0, 1 }),
	expr_str, advance_num, arg_idx)
{
	s << expr_str << " = x0\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto expr = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();

	auto iter = expr.begin();
	std::advance(iter, advance_num);

	BOOST_TEST(iter.parent_arg_idx() == arg_idx);
}


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
		// replacement
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


// given any expression, replacing the free variable IDs should be
// invariant to equivalence if and only if it doesn't reduce the
// number of free variable IDs
BOOST_DATA_TEST_CASE(replace_free_with_free_invariant_to_equivalence,
	boost::unit_test::data::make({
		"x", "*(x, *(y, z))", "*(*(x, y), *(z, w))",
		"e", "i(x)", "i(i(i(i(i(i(*(e, x)))))))" }) *
	boost::unit_test::data::xrange(0, 5) *
	boost::unit_test::data::xrange(0, 5), expr_str, id_from, id_to)
{
	s << expr_str << " = x\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto expr = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();

	auto id_set = expr.free_var_ids();

	bool should_be_equivalent = !(id_set.find(id_from) != id_set.end()
		&& id_set.find(id_to) != id_set.end() && id_from != id_to);

	auto replaced = expr.replace_free_with_free(id_from, id_to);

	BOOST_TEST(expr.equivalent(replaced) == should_be_equivalent);

	// in this case, they are equivalent if and only if they retained
	// the same number of free variables
	BOOST_TEST((expr.free_var_ids().size() ==
		replaced.free_var_ids().size()) == should_be_equivalent);
}


// data for the tests below
const char* identical_expr_data[] =
{
	"x", "*(x, *(y, z))", "*(*(x, y), *(z, w))",
	"e", "i(x)", "i(i(i(i(i(i(*(e, x)))))))"
};
BOOST_DATA_TEST_CASE(test_identical,
	boost::unit_test::data::make(identical_expr_data)*
	boost::unit_test::data::make(identical_expr_data),
	expr1_str, expr2_str)
{
	s << expr1_str << " = x\n";  // arbitrary RHS
	s << expr2_str << " = x\n";  // arbitrary RHS

	auto p_stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	auto expr1 = dynamic_cast<const Statement&>(p_stmts->at(0)).lhs();
	auto expr2 = dynamic_cast<const Statement&>(p_stmts->at(1)).lhs();

	const bool should_be_identical = (expr1_str == expr2_str);

	// check correct
	BOOST_TEST(expr1.identical(expr2) == should_be_identical);

	// check symmetric
	BOOST_TEST(expr1.identical(expr2) == expr2.identical(expr1));

	// check reflexive
	BOOST_TEST(expr1.identical(expr1));
}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


