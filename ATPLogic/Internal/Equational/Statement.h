#pragma once


/*

Statement.h

Implementation of the IStatement interface for equational logic. In
equational logic, the main idea is to try to deduce if two things are
equal using a set of equality rules given in a definition file.

If it comes the time to optimise the equational logic statements to
make search faster, here is the place to do it - at the moment the
Statement objects store the syntax trees internally. We do this
because it is simple and convenient, however it is inefficient.
Syntax trees are a good intermediate format (to pass between parsing
and the Statement object).

Note that an equational statement is trivially true if and only if it
is of the form "x = x", with some substitution for "x", or is equivalent
to one of the axioms in the equality rules. Thus, to check
if a statement is trivial, we check if the left hand side and right
hand side are identical (without allowing free variables to be
swapped; this is obvious because f(x,y) /= f(y,x) in general.)

*/


#include <memory>
#include <vector>
#include <string>
#include <map>
#include <boost/iterator/zip_iterator.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "SyntaxNodes.h"
#include "SyntaxTreeFold.h"


namespace atp
{
namespace logic
{
namespace equational
{


class KnowledgeKernel;  // forward definition


class ATP_LOGIC_API Statement : public IStatement
{
public:
	// precondition: !equational::needs_free_var_id_rebuild(p_root)
	// and p_root must be an eq node, with no other eq nodes in the
	// tree.
	Statement(const KnowledgeKernel& ker,
		SyntaxNodePtr p_root);
	Statement(const Statement& other);

	// the only reason this is here is so that we can put Statement
	// objects into a vector... try to avoid using this otherwise!
	Statement(Statement&& other) noexcept;

	// the only reason this is here is so that we can put Statement
	// objects into a vector... try to avoid using this otherwise!
	Statement& operator= (const Statement& other);

	// implemented by another fold
	std::string to_str() const override;

	inline size_t num_free_vars() const
	{
		return m_num_free_vars;
	}

	inline const KnowledgeKernel& kernel() const
	{
		return m_ker;
	}

	// perform a fold operation over the syntax tree
	template<typename ResultT, typename EqFuncT,
		typename FreeFuncT, typename ConstFuncT,
		typename FFuncT>
	ResultT fold(EqFuncT eq_func, FreeFuncT free_func,
		ConstFuncT const_func, FFuncT f_func) const
	{
		// just use the syntax tree fold
		return fold_syntax_tree<ResultT>(eq_func, free_func,
			const_func, f_func, m_root);
	}

	// perform a fold over a pair of syntax trees (where
	// there are five functions: one function for each type
	// of syntax node, which is invoked when both syntax
	// trees have the same type, then a default version where
	// the nodes have different type).
	// IMPORTANT NOTE: default_func is also called when we are
	// comparing two function nodes with different arity!
	template<typename ResultT, typename EqPairFuncT,
		typename FreePairFuncT, typename ConstPairFuncT,
		typename FuncPairFuncT, typename DefaultPairFuncT>
	ResultT fold_pair(EqPairFuncT eq_func, FreePairFuncT free_func,
		ConstPairFuncT const_func, FuncPairFuncT f_func,
		DefaultPairFuncT default_func, const Statement& other) const
	{
		// static assertions on the types of the functions given:

		static_assert(std::is_convertible<EqPairFuncT,
			std::function<ResultT(ResultT, ResultT)>>::value,
			"EqFuncT should be of type (ResultT, ResultT) -> ResultT");
		static_assert(std::is_convertible<FreePairFuncT,
			std::function<ResultT(size_t, size_t)>>::value,
			"FreeFuncT should be of type (size_t, size_t) -> ResultT");
		static_assert(std::is_convertible<ConstPairFuncT,
			std::function<ResultT(size_t, size_t)>>::value,
			"ConstFuncT should be of type (size_t, size_t) -> ResultT");
		static_assert(std::is_convertible<FuncPairFuncT,
			std::function<ResultT(size_t, size_t,
				typename std::list<ResultT>::iterator,
				typename std::list<ResultT>::iterator)>>::value,
			"FFuncT should be of type (size_t, size_t, "
			"std::list<ResultT>::iterator,"
			" std::list<ResultT>::iterator) -> ResultT");
		static_assert(std::is_convertible<DefaultPairFuncT,
			std::function<ResultT(SyntaxNodePtr, SyntaxNodePtr)>>::value,
			"EqFuncT should be of type (SyntaxNodePtr, "
			"SyntaxNodePtr) -> ResultT");

		// we will proceed similarly to the normal fold function,
		// but iterating over pairs instead of just singletons
		std::list<std::pair<SyntaxNodePtr, SyntaxNodePtr>> todo_stack;
		std::list<ResultT> result_stack;
		std::set<std::pair<SyntaxNodePtr, SyntaxNodePtr>> seen;

		todo_stack.push_back(std::make_pair(m_root, other.m_root));

		while (!todo_stack.empty())
		{
			auto pair = todo_stack.back();
			todo_stack.pop_back();

			const bool seen_pair = (seen.find(pair) != seen.end());
			
			if (pair.first->get_type() !=
				pair.second->get_type())
				result_stack.push_back(default_func(pair.first,
					pair.second));
			else switch (pair.first->get_type())
			{
			case SyntaxNodeType::EQ:
			{
				auto p_first = dynamic_cast<EqSyntaxNode*>(
					pair.first.get());
				auto p_second = dynamic_cast<EqSyntaxNode*>(
					pair.second.get());

				ATP_LOGIC_ASSERT(p_first != nullptr);
				ATP_LOGIC_ASSERT(p_second != nullptr);

				if (!seen_pair)
				{
					// push ourselves:
					todo_stack.push_back(pair);

					// examine children:

					todo_stack.push_back(std::make_pair(
						p_first->left(), p_second->left()));
					todo_stack.push_back(std::make_pair(
						p_first->right(), p_second->right()));

					// mark as seen
					seen.insert(pair);
				}
				else
				{
					ATP_LOGIC_ASSERT(result_stack.size() >= 2);

					auto left_result = result_stack.back();
					result_stack.pop_back();
					auto right_result = result_stack.back();
					result_stack.pop_back();

					result_stack.push_back(eq_func(left_result,
						right_result));
				}
			}
			break;
			case SyntaxNodeType::FREE:
			{
				auto p_first = dynamic_cast<FreeSyntaxNode*>(
					pair.first.get());
				auto p_second = dynamic_cast<FreeSyntaxNode*>(
					pair.second.get());

				ATP_LOGIC_ASSERT(p_first != nullptr);
				ATP_LOGIC_ASSERT(p_second != nullptr);

				result_stack.push_back(free_func(p_first->get_free_id(),
					p_second->get_free_id()));
			}
			break;
			case SyntaxNodeType::CONSTANT:
			{
				auto p_first = dynamic_cast<ConstantSyntaxNode*>(
					pair.first.get());
				auto p_second = dynamic_cast<ConstantSyntaxNode*>(
					pair.second.get());

				ATP_LOGIC_ASSERT(p_first != nullptr);
				ATP_LOGIC_ASSERT(p_second != nullptr);

				result_stack.push_back(const_func(
					p_first->get_symbol_id(),
					p_second->get_symbol_id()));
			}
			break;
			case SyntaxNodeType::FUNC:
			{
				auto p_first = dynamic_cast<FuncSyntaxNode*>(
					pair.first.get());
				auto p_second = dynamic_cast<FuncSyntaxNode*>(
					pair.second.get());

				ATP_LOGIC_ASSERT(p_first != nullptr);
				ATP_LOGIC_ASSERT(p_second != nullptr);

				// handle the annoying side case where the functions
				// have different arity:
				if (p_first->get_arity() != p_second->get_arity())
				{
					result_stack.push_back(default_func(
						pair.first, pair.second));
				}
				else if (!seen_pair)
				{
					// push ourselves:
					todo_stack.push_back(pair);

					// push child (argument) pairs to the
					// todo stack in reverse order!!

					std::transform(boost::make_zip_iterator(
						boost::make_tuple(p_first->rbegin(),
							p_second->rbegin())),
						boost::make_zip_iterator(boost::make_tuple(
							p_first->rend(), p_second->rend())),
						std::back_inserter(todo_stack),
						[](boost::tuple<SyntaxNodePtr,
							SyntaxNodePtr> tup)
						{ return std::make_pair(tup.get<0>(),
							tup.get<1>()); });

					// mark as seen
					seen.insert(pair);
				}
				else
				{
					// the results of our children are now at the
					// back of the stack

					ATP_LOGIC_ASSERT(p_first->get_arity() ==
						p_second->get_arity());
					ATP_LOGIC_ASSERT(result_stack.size() >=
						p_first->get_arity());

					auto result_rbegin_iter = result_stack.rbegin();
					std::advance(result_rbegin_iter,
						p_first->get_arity());
					auto result_begin_iter = result_rbegin_iter.base();

					// compute our result

					auto func_result = f_func(p_first->get_symbol_id(),
						p_second->get_symbol_id(),
						result_begin_iter, result_stack.end());

					// erase child results from stack
					result_stack.erase(result_begin_iter,
						result_stack.end());

					// add our result to the stack
					result_stack.push_back(func_result);
				}
			}
			break;
			}
		}

		// we should only have one value left, which would be due to the
		// very first node we inserted:
		ATP_LOGIC_ASSERT(result_stack.size() == 1);
		return result_stack.front();
	}

private:
	const KnowledgeKernel& m_ker;
	SyntaxNodePtr m_root;
	size_t m_num_free_vars;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


