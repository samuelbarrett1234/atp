/**

\file

\author Samuel Barrett

\brief This suite tests `equational::ExprTreeFlyweight`

*/


#include <Internal/Equational/ExprTreeFlyweight.h>
#include "../Test.h"


using atp::logic::equational::ExprTreeFlyweight;
using atp::logic::equational::SyntaxNodeType;


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_AUTO_TEST_SUITE(ExprTreeFlyweightTests);


BOOST_AUTO_TEST_CASE(test_merge_from)
{
	ExprTreeFlyweight fw1, fw2;

	size_t children[] = { 2, 7, 9 };
	SyntaxNodeType child_types[] = { SyntaxNodeType::CONSTANT,
		SyntaxNodeType::FREE, SyntaxNodeType::CONSTANT };

	fw1.add_func(8, 1, children, children + 1,
		child_types, child_types + 1);
	fw1.set_root(0, SyntaxNodeType::FUNC);

	fw2.add_func(9, 2, children + 1, children + 3,
		child_types + 1, child_types + 3);
	fw2.set_root(0, SyntaxNodeType::FUNC);

	const size_t fw2_new_root = fw1.merge_from(fw2);

	BOOST_TEST(fw1.size() == 2);
	BOOST_TEST(fw2_new_root == 1);
	BOOST_TEST(fw1.func_symb_id(0) == 8);
	BOOST_TEST(fw1.func_symb_id(1) == 9);
	BOOST_TEST(fw1.func_children(0).at(0) == 2);
	BOOST_TEST(fw1.func_children(1).at(0) == 7);
	BOOST_TEST(fw1.func_children(1).at(1) == 9);
}


BOOST_AUTO_TEST_CASE(test_merge_from_with_different_sizes)
{
	ExprTreeFlyweight fw1, fw2;

	size_t children[] = { 2, 7 };
	SyntaxNodeType child_types[] = { SyntaxNodeType::CONSTANT,
		SyntaxNodeType::FREE };

	fw1.add_func(8, 1, children, children + 1,
		child_types, child_types + 1);
	fw1.set_root(0, SyntaxNodeType::FUNC);

	size_t func_id = fw2.add_func(9, 1, children + 1,
		children + 2, child_types + 1, child_types + 2);

	size_t func_children[] = { func_id };
	SyntaxNodeType func_child_types[] = { SyntaxNodeType::FUNC };
	func_id = fw2.add_func(6, 1, func_children, func_children + 1,
		func_child_types, func_child_types + 1);
	fw2.set_root(func_id, SyntaxNodeType::FUNC);
	
	const size_t fw2_new_root = fw1.merge_from(fw2);

	BOOST_TEST(fw1.size() == 3);
	BOOST_TEST(fw2_new_root == func_id + 1);
	BOOST_TEST(fw1.func_symb_id(0) == 8);
	BOOST_TEST(fw1.func_symb_id(fw2_new_root) == 6);
	// fw2_new_root either 1 or 2, so 3-x gives us 2 or 1
	BOOST_TEST(fw1.func_symb_id(3 - fw2_new_root) == 9);
	BOOST_TEST(fw1.func_children(fw2_new_root).at(0)
		== 3 - fw2_new_root);
}


BOOST_AUTO_TEST_CASE(test_setting_root_clears_when_not_func)
{
	ExprTreeFlyweight fw;

	size_t child[] = { 2 };
	SyntaxNodeType child_type[] = { SyntaxNodeType::CONSTANT };

	fw.add_func(4, 1, child, child + 1, child_type, child_type + 1);
	fw.set_root(0, SyntaxNodeType::FUNC);

	BOOST_TEST(fw.size() == 1);

	fw.set_root(7, SyntaxNodeType::CONSTANT);

	BOOST_TEST(fw.size() == 0);
}


BOOST_AUTO_TEST_CASE(test_copy)
{
	ExprTreeFlyweight fw1, fw2;

	size_t child[] = { 2 };
	SyntaxNodeType child_type[] = { SyntaxNodeType::CONSTANT };

	fw1.add_func(4, 1, child, child + 1, child_type, child_type + 1);
	fw1.set_root(0, SyntaxNodeType::FUNC);

	fw2 = fw1;

	BOOST_TEST(fw2.size() == 1);
	BOOST_TEST(fw2.func_symb_id(0) == 4);
	BOOST_TEST(fw2.func_children(0).at(0) == 2);
}


BOOST_AUTO_TEST_CASE(test_move)
{
	ExprTreeFlyweight fw1, fw2;

	size_t child[] = { 2 };
	SyntaxNodeType child_type[] = { SyntaxNodeType::CONSTANT };

	fw1.add_func(4, 1, child, child + 1, child_type, child_type + 1);
	fw1.set_root(0, SyntaxNodeType::FUNC);

	fw2 = std::move(fw1);

	BOOST_TEST(fw2.size() == 1);
	BOOST_TEST(fw2.func_symb_id(0) == 4);
	BOOST_TEST(fw2.func_children(0).at(0) == 2);
}


BOOST_AUTO_TEST_SUITE_END();  // ExprTreeFlyweightTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


