/**

\file

\author SamuelBarrett

\details This file tests the conversion of parse trees to syntax
    trees; in particular, it tests the type-checking based on the
	definitions given in the knowledge kernel.

*/


#include <sstream>
#include <string>
#include <Internal/Equational/Parser.h>
#include <Internal/Equational/SyntaxNodes.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include "../Test.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::EqSyntaxNode;
using atp::logic::equational::FuncSyntaxNode;
using atp::logic::equational::ConstantSyntaxNode;
using atp::logic::equational::FreeSyntaxNode;
using atp::logic::equational::parse_statements;
using atp::logic::equational::ptree_to_stree;


namespace atp {
namespace logic {
namespace equational {
static inline std::ostream& boost_test_print_type (std::ostream& os,
	SyntaxNodeType type)
{
	switch (type)
	{
	case SyntaxNodeType::EQ:
		os << "EQ";
		break;
	case SyntaxNodeType::FREE:
		os << "FREE";
		break;
	case SyntaxNodeType::CONSTANT:
		os << "CONSTANT";
		break;
	case SyntaxNodeType::FUNC:
		os << "FUNC";
		break;
	}
	return os;
}
}  // namespace equational
}  // namespace logic
}  // namespace atp


struct ParseTreeToSyntaxTreeFixture
{
	ParseTreeToSyntaxTreeFixture()
	{
		// group theory definitions - why not:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
		s << std::noskipws;
	}

	std::stringstream s;
	KnowledgeKernel ker;
};


// a big list of statements to check whether or not they are valid
// insofar as they wouldn't cause a syntax / type error (assuming
// they are all parsable!)
static std::string stmts_to_check_validity[] =
{
	// here, f is a free function (not been defined in the kernel)
	// and this statement basically says that: "all functions in two
	// variables are symmetric in their arguments", however as of yet
	// we don't allow universal quantification over functions.
	"f(x, y) = f(y, x)",

	// some valid statements; note that we are only type-checking
	// here. of course these statements may or may not actually be
	// true in group theory!
	"x = y", "*(x, y) = *(y, x)",
	"*(i(x), i(y)) = i(*(y, x))",
	"i(i(x)) = x", "*(x, e) = x",

	// some invalid statements
	"e(x) = x", "*(x, i) = x",
	"*(x, y, z) = z", "i(*) = *"
};
static bool stmts_is_valid[] =
{
	false,

	true, true,
	true,
	true, true,

	false, false,
	false, false
};


// checking the syntax node types of the lhs and rhs of the equations
// (note that it refers to the type of the "uppermost" node on the
// side).
static std::string stmts_to_check_type[] =
{
	"x = y", "i(x) = x", "e = *(x, y)"
};
static SyntaxNodeType lhs_types[] =
{
	SyntaxNodeType::FREE, SyntaxNodeType::FUNC,
	SyntaxNodeType::CONSTANT
};
static SyntaxNodeType rhs_types[] =
{
	SyntaxNodeType::FREE, SyntaxNodeType::FREE,
	SyntaxNodeType::FUNC
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ParseTreeToSyntaxTreeTests,
	ParseTreeToSyntaxTreeFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/ParseStatementsTests")
	*boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests"));


BOOST_DATA_TEST_CASE(test_validity,
	boost::unit_test::data::make(stmts_to_check_validity)
	^ boost::unit_test::data::make(stmts_is_valid), stmt, is_valid)
{
	s << stmt;
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	// may or may not return nullptr, depending on the expected
	// result given in the second element of the pair stmt_and_valid:
	auto syntax_tree = ptree_to_stree(
		parse_tree, ker);

	BOOST_TEST((syntax_tree != nullptr)
		== is_valid);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/ParseTreeToSyntaxTreeTests/test_validity"))
BOOST_DATA_TEST_CASE(test_lhs_rhs_types,
	boost::unit_test::data::make(stmts_to_check_type)
	^ boost::unit_test::data::make(lhs_types)
	^ boost::unit_test::data::make(rhs_types),
	stmt, lhs_type, rhs_type)
{
	s << stmt;
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ker);

	// it should've been type correct:
	BOOST_REQUIRE(syntax_tree != nullptr);

	// root node should be =
	BOOST_REQUIRE(syntax_tree->get_type() ==
		SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(syntax_tree.get());

	BOOST_REQUIRE(p_eq != nullptr);

	auto p_left = p_eq->left();
	auto p_right = p_eq->right();

	BOOST_TEST(p_left->get_type() == lhs_type);
	BOOST_TEST(p_right->get_type() == rhs_type);
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/ParseTreeToSyntaxTreeTests/test_lhs_rhs_types"))
BOOST_AUTO_TEST_CASE(test_func_args_typed_correctly)
{
	// here we will do a nitty-gritty analysis of the syntax tree
	// resulting from the following expression, to ensure it's
	// totally correct:
	s << "*(x, i(x)) = e";
	auto result = parse_statements(s);

	// the tests should be parsable at least:
	BOOST_REQUIRE(result.has_value());

	// only one statement per test please:
	BOOST_REQUIRE(result.get().size() == 1);

	auto parse_tree = result.get().front();

	auto syntax_tree = ptree_to_stree(
		parse_tree, ker);

	// it should've been type correct:
	BOOST_REQUIRE(syntax_tree != nullptr);

	// root node should be =
	BOOST_REQUIRE(syntax_tree->get_type() ==
		SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(syntax_tree.get());

	BOOST_REQUIRE(p_eq != nullptr);

	auto p_left = p_eq->left();
	auto p_right = p_eq->right();

	BOOST_TEST(p_left->get_type() ==
		SyntaxNodeType::FUNC);
	BOOST_TEST(p_right->get_type() ==
		SyntaxNodeType::CONSTANT);

	auto p_func = dynamic_cast<FuncSyntaxNode*>(p_left.get());
	auto p_const = dynamic_cast<ConstantSyntaxNode*>(p_right.get());

	BOOST_REQUIRE(p_func != nullptr);
	BOOST_REQUIRE(p_const != nullptr);

	BOOST_TEST(p_const->get_symbol_id() == ker.symbol_id("e"));

	BOOST_TEST(p_func->get_symbol_id() == ker.symbol_id("*"));

	BOOST_TEST(p_func->get_arity() == 2);

	BOOST_TEST(std::distance(p_func->begin(),
		p_func->end()) == 2);

	auto iter = p_func->begin();

	auto left_arg = *iter;
	std::advance(iter, 1);
	auto right_arg = *iter;

	BOOST_TEST(left_arg->get_type() == SyntaxNodeType::FREE);
	BOOST_TEST(right_arg->get_type() == SyntaxNodeType::FUNC);

	auto p_left_arg = dynamic_cast<FreeSyntaxNode*>(left_arg.get());
	auto p_right_arg = dynamic_cast<FuncSyntaxNode*>(
		right_arg.get());

	BOOST_REQUIRE(p_left_arg != nullptr);
	BOOST_REQUIRE(p_right_arg != nullptr);

	// we will test more on free IDs later, but if there is exactly
	// one free variable in a statement, it must have free ID 0
	BOOST_TEST(p_left_arg->get_free_id() == 0);

	BOOST_TEST(p_right_arg->get_symbol_id() == ker.symbol_id("i"));

	BOOST_REQUIRE(p_right_arg->get_arity() == 1);

	BOOST_REQUIRE(std::distance(p_right_arg->begin(),
		p_right_arg->end()) == 1);

	iter = p_right_arg->begin();
	auto final_child = *iter;

	BOOST_REQUIRE(final_child->get_type() == SyntaxNodeType::FREE);

	auto p_final_child = dynamic_cast<FreeSyntaxNode*>(
		final_child.get());

	BOOST_REQUIRE(p_final_child != nullptr);

	// it is the same free variable as before:
	BOOST_TEST(p_final_child->get_free_id() == 0);
}


BOOST_AUTO_TEST_SUITE_END();  // ParseTreeToSyntaxTreeTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


