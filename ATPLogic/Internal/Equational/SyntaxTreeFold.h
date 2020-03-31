#pragma once


/*

EquationalSyntaxTreeFold.h

This file contains a (templated) implementation of a fold over syntax
trees. Folds are a special kind of concept, prominent in functional
programming. Applying a fold to a tree like this one can be a useful
way of computing/aggregating information over the tree without having
to write boilerplate code.

A fold basically works by specifying a function to apply for each
kind of node, and the fold then handles the rest of the recursion for
you.

However, recursion is inefficient, so we use a stack instead. In
particular, we use two stacks: one for keeping track of which nodes
we are yet to examine, and which results we are yet to use. Some
nodes need to be examined twice (the first time, we push its children
onto the stack. the second time, we combine the results of its
children and push our result to the results_stack) thus we have a set
of seen nodes too.

*/


#include <set>
#include <list>
#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


// perform a fold on syntax trees!
// this is very handy for computing functions of the trees, and just
// traversing the trees in general.
// EqFuncT : should be of type (ResultT, ResultT) -> ResultT
// FreeFuncT : should be of type (size_t) -> ResultT
// ConstFuncT : should be of type (size_t) -> ResultT
// FFuncT : should be of type (size_t, std::list<ResultT>::iterator,
//                             std::list<ResultT>::iterator) -> ResultT
template<typename ResultT, typename EqFuncT,
typename FreeFuncT, typename ConstFuncT, typename FFuncT>
ResultT fold_syntax_tree(EqFuncT eq_func, FreeFuncT free_func,
	ConstFuncT const_func, FFuncT f_func, SyntaxNodePtr p_root
)
{
	std::list<typename ResultT> result_stack;
	std::list<ISyntaxNode*> todo_stack;
	std::set<ISyntaxNode*> seen;

	todo_stack.push_back(p_root.get());

	while (!todo_stack.empty())
	{
		ISyntaxNode* p_node = todo_stack.back();
		todo_stack.pop_back();

		// first occurrence
		if (seen.find(p_node) == seen.end())
		{
			apply_to_syntax_node(
				// if EQ...
				[&todo_stack, &seen](EqSyntaxNode& eq)
				{
					// revisit it later
					todo_stack.push_back(&eq);
					seen.insert(&eq);

					// add children
					todo_stack.push_back(eq.left());
					todo_stack.push_back(eq.right());
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
				[&todo_stack, &seen](FuncSyntaxNode& f)
				{
					// revisit it later
					todo_stack.push_back(p_node);
					seen.insert(p_node);

					// add children
					todo_stack.insert(todo_stack.end(),
						f.begin(), f.end());
				},
				*p_node
			);
		}
		else  // else revisiting
		{
			apply_to_syntax_node(
				// if EQ...
				[&result_stack, &eq_func](EqSyntaxNode& eq)
				{
					ATP_LOGIC_ASSERT(result_stack.size() >= 2);

					auto right_result = result_stack.back();
					result_stack.pop_back();
					auto left_result = result_stack.back();
					result_stack.pop_back();

					// compute function of eq for its children:
					result_stack.push_back(eq_func(left_result,
						right_result));
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

					// find list of child results:
					auto result_iter = result_stack.rbegin();
					std::advance(result_iter, f.get_arity());

					// now compute result for p_func:
					auto result = f_func(f.get_symbol_id(),
						result_iter.base(), result_stack.end());

					// now remove the child results and add ours:
					result_stack.erase(result_iter.base(),
						result_stack.end());
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


