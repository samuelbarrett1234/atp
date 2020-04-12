#pragma once


/**

\file

\author Samuel Barrett

\brief Provides a templated implementation of a fold over parse trees


*/


#include <set>
#include <list>
#include <string>
#include <functional>
#include <algorithm>
#include <type_traits>
#include "ParseNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**

\brief Perform a fold operation over a given parse tree

\tparam ResultT The return type of this operation.

\tparam EqFuncT The function to use as the equality node constructor,
    which must have type ResultT x ResultT -> ResultT

\tparam IdentifierFuncT The function to use as the identifier node
    constructor, which must have type std::string x
	std::vector<ResultT>::iterator x std::vector<ResultT>::iterator
	-> ResultT

\details Folds are a special kind of concept, prominent in
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
	typename IdentifierFuncT>
ResultT fold_parse_tree(EqFuncT eq_func,
	IdentifierFuncT identifier_func,
	ParseNodePtr p_root)
{
	ATP_LOGIC_PRECOND(p_root != nullptr);

	// static assertions on the types of the functions given:

	static_assert(std::is_convertible<EqFuncT,
		std::function<ResultT(ResultT,
			ResultT)>>::value,
		"EqFuncT should be of type (ResultT, ResultT) -> ResultT");
	static_assert(std::is_convertible<IdentifierFuncT,
		std::function<ResultT(std::string,
			typename std::vector<ResultT>::iterator,
			typename std::vector<ResultT>::iterator)>>::value,
		"IdentifierFuncT should be of type (std::string, "
		"std::vector<ResultT>::iterator,"
		" std::vector<ResultT>::iterator) -> ResultT");
	
	// now proceed with the function:

	std::vector<ResultT> result_stack;
	std::vector<IParseNode*> todo_stack;
	std::vector<bool> seen_stack;

	todo_stack.push_back(p_root.get());
	seen_stack.push_back(false);

	while (!todo_stack.empty())
	{
		ATP_LOGIC_ASSERT(seen_stack.size() == todo_stack.size());

		IParseNode* p_node = todo_stack.back();
		todo_stack.pop_back();
		const bool seen = seen_stack.back();
		seen_stack.pop_back();

		// first occurences:
		if (!seen)
		{
			switch (p_node->get_type())
			{
			case ParseNodeType::EQ:
			{
				EqParseNode* p_eq =
					dynamic_cast<EqParseNode*>(p_node);

				// revisit it later
				todo_stack.push_back(p_node);
				seen_stack.push_back(true);

				// add children
				todo_stack.push_back(p_eq->left().get());
				todo_stack.push_back(p_eq->right().get());
				seen_stack.push_back(false);
				seen_stack.push_back(false);
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

					std::vector<ResultT> empty_list;

					result_stack.emplace_back(
						identifier_func(p_id->get_name(),
							empty_list.begin(), empty_list.end())
					);
				}
				else
				{
					// re-add ourselves and check again later:
					todo_stack.push_back(p_node);
					seen_stack.push_back(true);

					// add children in reverse order:
					std::transform(p_id->rbegin(), p_id->rend(),
						std::back_inserter(todo_stack),
						[](const ParseNodePtr& p_node)
						{ return p_node.get(); });

					// extend by "arity", taking values "false"
					seen_stack.resize(seen_stack.size() + arity,
						false);
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

				/*
				Note that we have to pop left and right in this
				order, because we initially pushed left then
				right, but since then, they have both been popped
				from the todo_stack and pushed to the
				result_stack thus their order has been inverted
				(so: inverted twice means left the same!)
				[see the unit tests for more].
				*/
				ResultT left_result = result_stack.back();
				result_stack.pop_back();
				ResultT right_result = result_stack.back();
				result_stack.pop_back();

				// compute function of eq for its children:
				result_stack.emplace_back(eq_func(left_result,
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

#ifdef ATP_LOGIC_DEFENSIVE
				const size_t size_before = result_stack.size();
#endif

				// find list of child results:
				auto result_iter = result_stack.rbegin();
				std::advance(result_iter, arity);

				// now compute result for p_func:
				auto result = identifier_func(p_id->get_name(),
					result_iter.base(), result_stack.end());

				// now remove the child results and add ours:
				result_stack.erase(result_iter.base(),
					result_stack.end());

#ifdef ATP_LOGIC_DEFENSIVE
				ATP_LOGIC_ASSERT(result_stack.size() + arity
					== size_before);
#endif

				result_stack.emplace_back(std::move(result));
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


