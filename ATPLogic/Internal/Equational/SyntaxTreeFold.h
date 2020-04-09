#pragma once


/**

\file

\author Samuel Barrett

\brief Provides a templated implementation of a fold over syntax
    trees.

*/


#include <list>
#include <vector>
#include <functional>
#include <type_traits>
#include <algorithm>
#include "SyntaxNodes.h"
#include "SyntaxTreeTraversal.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**

\brief Perform a fold operation over a given syntax tree

\tparam ResultT The return type of this operation.

\tparam EqFuncT The function to use as the equality node constructor,
	which must have type ResultT x ResultT -> ResultT

\tparam FreeFuncT The function to use as the free variable node
    constructor, with type size_t -> ResultT

\tparam ConstFuncT The function to use as the constant node
	constructor, with type size_t -> ResultT

\tparam FFuncT The function to use as the function node constructor,
    with type size_t x std::list<ResultT>::iterator x
    std::list<ResultT>::iterator -> ResultT

\detailed Folds are a special kind of concept, prominent in
	functional programming. Applying a fold to a tree like this one
	can be a useful way of computing/aggregating information over the
	tree without having to write boilerplate code. A fold basically
	works by specifying a function to apply for each kind of node, and
	the fold then handles the rest of the recursion for you. However,
	recursion is inefficient, so we use a stack instead. In
	particular, we use two stacks: one for keeping track of which
	nodes we are yet to examine, and which results we are yet to use.
	Some nodes need to be examined twice (the first time, we push its
	children onto the stack. the second time, we combine the results
	of its children and push our result to the results_stack) thus we
	have a set of seen nodes too.
*/
template<typename ResultT, typename EqFuncT,
typename FreeFuncT, typename ConstFuncT, typename FFuncT>
ResultT fold_syntax_tree(EqFuncT eq_func, FreeFuncT free_func,
	ConstFuncT const_func, FFuncT f_func, SyntaxNodePtr p_root
)
{
	ATP_LOGIC_PRECOND(p_root != nullptr);

	// static assertions on the types of the functions given:

	static_assert(std::is_convertible<EqFuncT,
		std::function<ResultT(ResultT, ResultT)>>::value,
		"EqFuncT should be of type (ResultT, ResultT) -> ResultT");
	static_assert(std::is_convertible<FreeFuncT,
		std::function<ResultT(size_t)>>::value,
		"FreeFuncT should be of type (size_t) -> ResultT");
	static_assert(std::is_convertible<ConstFuncT,
		std::function<ResultT(size_t)>>::value,
		"ConstFuncT should be of type (size_t) -> ResultT");
	static_assert(std::is_convertible<FFuncT,
		std::function<ResultT(size_t,
			typename std::list<ResultT>::iterator,
			typename std::list<ResultT>::iterator)>>::value,
		"FFuncT should be of type (size_t, "
		"std::list<ResultT>::iterator,"
		" std::list<ResultT>::iterator) -> ResultT");

	// now proceed with the function:

	std::list<ResultT> result_stack;
	std::list<ISyntaxNode*> todo_stack;
	std::vector<bool> seen_stack;

	todo_stack.push_back(p_root.get());
	seen_stack.push_back(false);

	while (!todo_stack.empty())
	{
		ATP_LOGIC_ASSERT(seen_stack.size() ==
			todo_stack.size());

		ISyntaxNode* p_node = todo_stack.back();
		todo_stack.pop_back();
		const bool seen = seen_stack.back();
		seen_stack.pop_back();

		// first occurrence
		if (!seen)
		{
			apply_to_syntax_node<void>(
				// if EQ...
				[&todo_stack, &seen_stack](EqSyntaxNode& eq)
				{
					// revisit it later
					todo_stack.push_back(&eq);
					seen_stack.push_back(true);

					// add children
					todo_stack.push_back(eq.left().get());
					seen_stack.push_back(false);
					todo_stack.push_back(eq.right().get());
					seen_stack.push_back(false);
				},
				// if FREE...
				[&result_stack, &free_func](FreeSyntaxNode& free)
				{
					// compute result and add it to result stack:
					result_stack.push_back(free_func(
						free.get_free_id()));
				},
				// if CONST...
				[&result_stack, &const_func](ConstantSyntaxNode& c)
				{
					// compute result and add it to result stack:
					result_stack.push_back(const_func(
						c.get_symbol_id()));
				},
				// if FUNC...
				[&todo_stack, &seen_stack](FuncSyntaxNode& f)
				{
					// revisit it later
					todo_stack.push_back(&f);
					seen_stack.push_back(true);

					// add children in reverse order
					std::transform(f.rbegin(), f.rend(),
						std::back_inserter(todo_stack),
						[](SyntaxNodePtr p_node)
						{ return p_node.get(); });
					seen_stack.resize(seen_stack.size()
						+ f.get_arity(), false);
				},
				*p_node
			);
		}
		else  // else revisiting
		{
			apply_to_syntax_node<void>(
				// if EQ...
				[&result_stack, &eq_func](EqSyntaxNode& eq)
				{
					ATP_LOGIC_ASSERT(result_stack.size() >= 2);

					/*
					Note that we have to pop left and right in this
					order, because we initially pushed left then
					right, but since then, they have both been popped
					from the todo_stack and pushed to the
					result_stack thus their order has been inverted
					(so: inverted twice means left the same!)
					[see the unit tests for more].
					*/
					auto left_result = result_stack.back();
					result_stack.pop_back();
					auto right_result = result_stack.back();
					result_stack.pop_back();

					// compute function of eq for its children:
					result_stack.push_back(eq_func(
						std::move(left_result),
						std::move(right_result)));
				},
				// if FREE...
				[](FreeSyntaxNode&) { },
				// if CONST...
				[](ConstantSyntaxNode&) { },
				// if FUNC...
				[&result_stack, &f_func](FuncSyntaxNode& f)
				{
					ATP_LOGIC_ASSERT(result_stack.size() >=
						f.get_arity());

#ifdef ATP_LOGIC_DEFENSIVE
					const size_t size_before = result_stack.size();
#endif

					// find list of child results:
					auto result_iter = result_stack.rbegin();
					std::advance(result_iter, f.get_arity());

					ATP_LOGIC_ASSERT(std::distance(result_iter.base(),
						result_stack.end()) == f.get_arity());

					// now compute result for p_func:
					auto result = f_func(f.get_symbol_id(),
						result_iter.base(),
						result_stack.end());

					// now remove the child results and add ours:
					result_stack.erase(result_iter.base(),
						result_stack.end());

#ifdef ATP_LOGIC_DEFENSIVE
					ATP_LOGIC_ASSERT(result_stack.size() + f.get_arity()
						== size_before);
#endif

					result_stack.push_back(result);
				},
				*p_node
			);
		}
	}

	// we should only have one value left, which would be due to the
	// very first node we inserted:
	ATP_LOGIC_ASSERT(result_stack.size() == 1);
	return result_stack.front();
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


