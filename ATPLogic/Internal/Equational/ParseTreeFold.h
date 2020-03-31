#pragma once


/*

EquationalParseTreeFold.h

This file contains a (templated) implementation of a fold over parse
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
#include "ParseNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


// perform a fold on parse trees!
// this is very handy for computing functions of the trees, and just
// traversing the trees in general.
// EqFuncT : should be of type (ResultT, ResultT) -> ResultT
// IdentifierFuncT : should be of type (string,
//                   std::list<ResultT>::iterator,
//                   std::list<ResultT>::iterator)
template<typename ResultT, typename EqFuncT,
	typename IdentifierFuncT>
ResultT fold_parse_tree(EqFuncT eq_func,
	IdentifierFuncT identifier_func,
	ParseNodePtr p_root)
{
	std::list<ResultT> result_stack;
	std::list<IParseNode*> todo_stack;
	std::set<IParseNode*> seen;

	todo_stack.push_back(p_root.get());

	while (!todo_stack.empty())
	{
		IParseNode* p_node = todo_stack.back();
		todo_stack.pop_back();

		// first occurences:
		if (seen.find(p_node) == seen.end())
		{
			switch (p_node->get_type())
			{
			case ParseNodeType::EQ:
			{
				EqParseNode* p_eq =
					dynamic_cast<EqParseNode*>(p_node);

				// revisit it later
				todo_stack.push_back(p_node);
				seen.insert(p_node);

				// add children
				todo_stack.push_back(eq.left());
				todo_stack.push_back(eq.right());
			}
				break;
			case ParseNodeType::IDENTIFIER:
			{
				IdentifierParseNode* p_id =
					dynamic_cast<IdentifierParseNode*>(p_node);

				// whether we recurse or not now depends on arity:
				const size_t arity = std::distance(p_id->begin(),
					p_id->end());

				if (arity == 0)
				{
					// handle now:

					std::list<typename ResultT> empty_list;

					result_stack.push_back(
						identifier_func(p_id->get_name(),
							empty_list.begin(), empty_list.end())
					);
				}
				else
				{
					// re-add ourselves and check again later:
					todo_stack.push_back(p_node);
					seen.insert(p_node);

					// add children:
					todo_stack.insert(todo_stack.end(),
						p_id->begin(), p_id->end());
				}
			}
			}
		}
		else
		{
			switch (p_node->get_type())
			{
			case ParseNodeType::EQ:
			{
				EqParseNode* p_eq =
					dynamic_cast<EqParseNode*>(p_node);

				ATP_LOGIC_ASSERT(result_stack.size() >= 2);

				auto right_result = result_stack.back();
				result_stack.pop_back();
				auto left_result = result_stack.back();
				result_stack.pop_back();

				// compute function of eq for its children:
				result_stack.push_back(eq_func(left_result,
					right_result));
			}
			break;
			case ParseNodeType::IDENTIFIER:
			{
				IdentifierParseNode* p_id =
					dynamic_cast<IdentifierParseNode*>(p_node);

				// whether we recurse or not now depends on arity:
				const size_t arity = std::distance(p_id->begin(),
					p_id->end());

				ATP_LOGIC_ASSERT(arity > 0);
				ATP_LOGIC_ASSERT(result_stack.size() >= arity);

				// find list of child results:
				auto result_iter = result_stack.rbegin();
				std::advance(result_iter, arity);

				// now compute result for p_func:
				auto result = identifier_func(p_id->get_name(),
					result_iter.base(), result_stack.end());

				// now remove the child results and add ours:
				result_stack.erase(result_iter.base(),
					result_stack.end());
				result_stack.push_back(result);
			}
			}
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


