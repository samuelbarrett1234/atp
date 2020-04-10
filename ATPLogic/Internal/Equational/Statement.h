#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the implementation of the IStatement interface for
    equational logic.

\todo If it comes the time to optimise the equational logic
    statements to make search faster, here is a good place to do it.
	At the moment the Statement objects store the syntax trees
	internally. We do this because it is simple and convenient, however
	it is inefficient. Syntax trees are a good intermediate format
	(to pass between parsing and the Statement object).

\note An equational statement is trivially true if and only if it
    is of the form "x = x", with some substitution for "x", or is
	equivalent to one of the axioms in the equality rules. Thus, to
	check if a statement is trivial, we check if the left hand side
	and right hand side are identical (without allowing free
	variables to be swapped; this is obvious because f(x,y) /= f(y,x)
	in general.)

*/


#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <boost/iterator/zip_iterator.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "SyntaxNodes.h"
#include "SyntaxTreeFold.h"


/**

\namespace atp::logic::equational

\brief The namespace for the equational logic implementation

*/


namespace atp
{
namespace logic
{
namespace equational
{


class KnowledgeKernel;  // forward definition


/**
Implementation of the IStatement interface for equational logic.

\todo This class definitely needs some optimisation.

\note As much as this class is supposed to be immutable, in order
    to be usable in standard containers like std::vector, it needs
	a copy constructor and an assignment operator.
*/
class ATP_LOGIC_API Statement : public IStatement
{
public:
	/**
	\pre p_root->get_type() == SyntaxNodeType::EQ
	*/
	Statement(const KnowledgeKernel& ker,
		SyntaxNodePtr p_root);
	Statement(const Statement& other);

	// the only reason this is here is so that we can put Statement
	// objects into a vector... try to avoid using this otherwise!
	Statement(Statement&& other) noexcept;

	// the only reason this is here is so that we can put Statement
	// objects into a vector... try to avoid using this otherwise!
	Statement& operator= (const Statement& other);

	// implemented by a fold
	std::string to_str() const override;

	/**
	\brief Transpose the statement (flip it around the equals sign)

	\details This can be implemented using a fold, but doing so is
	    very slow as it requires reallocating and rebuilding the
		whole tree.
	*/
	Statement transpose() const;

	/**
	\brief Get the LHS and RHS of this statement

	\returns A pair (LHS, RHS)

	\details This can be implemented using a fold, but doing so is
		very slow as it requires reallocating and rebuilding the
		whole tree.
	*/
	std::pair<SyntaxNodePtr, SyntaxNodePtr> get_sides() const;

	inline size_t num_free_vars() const
	{
		return m_free_var_ids.size();
	}

	inline const std::set<size_t>& free_var_ids() const
	{
		return m_free_var_ids;
	}

	inline const KnowledgeKernel& kernel() const
	{
		return m_ker;
	}

	/**
	\brief create a new statement obtained by replacing the RHS of
	    this statement with the RHS of the `other` statement.
	*/
	Statement adjoin_rhs(const Statement& other) const;

	/**
	\see atp::logic::equational::fold_syntax_tree
	*/
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

	/**
	\brief Perform a fold over pairs of syntax trees, where the fold
	    constructor is dependent on the node types of the pair. There
		are functions for pairs of matching types, and then a default
		function for when the pairs do not match.

	\tparam ResultT The return type (all functions must return this
	    type.)
	
	\tparam EqPairFuncT The function operating on pairs of equality
	    nodes, must have signature `ResultT x ResultT -> ResultT`

	\tparam FreePairFuncT The function operating on pairs of free
	    variable nodes, must have signature
		`size_t x size_t -> ResultT` where the two given integers are
		the free variable IDs in the pair.

	\tparam ConstPairFuncT The function operating on pairs of
	    constants, must have signature `size_t x size_t -> ResultT`
		where the two integers are the symbol IDs of the constants

	\tparam FuncPairFuncT The function operating on pairs of function
	    nodes, must have signature `size_t x size_t x std::list<
		ResultT>::iterator x std::list<ResultT>::iterator -> ResultT`
		where the two integers are the symbol IDs of the functions
		and the two iterators are the begin and end iterators of the
		function arguments' results.

	\tparam DefaultPairFuncT This is called when a pair of nodes are
	    encountered but don't have the same type, and it must have
		signature `SyntaxNodePtr x SyntaxNodePtr -> ResultT`, where the
		two arguments are just the two nodes.

	\param other The other statement to fold with, which can be
	    thought of as appearing on the RHS of the pair fold.
	*/
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
		std::vector<bool> seen_stack;

		todo_stack.push_back(std::make_pair(m_root, other.m_root));
		seen_stack.push_back(false);

		while (!todo_stack.empty())
		{
			ATP_LOGIC_ASSERT(seen_stack.size() ==
				todo_stack.size());

			auto pair = todo_stack.back();
			todo_stack.pop_back();
			const bool seen_pair = seen_stack.back();
			seen_stack.pop_back();

			if (pair.first->get_type() !=
				pair.second->get_type())
			{
				result_stack.push_back(default_func(pair.first,
					pair.second));
			}
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
					seen_stack.push_back(true);

					// examine children:

					todo_stack.push_back(std::make_pair(
						p_first->left(), p_second->left()));
					seen_stack.push_back(false);
					todo_stack.push_back(std::make_pair(
						p_first->right(), p_second->right()));
					seen_stack.push_back(false);
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
					seen_stack.push_back(true);

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

					seen_stack.resize(seen_stack.size() +
						p_first->get_arity(), false);
				}
				else
				{
					// the results of our children are now at the
					// back of the stack

					ATP_LOGIC_ASSERT(p_first->get_arity() ==
						p_second->get_arity());
					ATP_LOGIC_ASSERT(result_stack.size() >=
						p_first->get_arity());

#ifdef ATP_LOGIC_DEFENSIVE
					const size_t size_before = result_stack.size();
#endif

					auto result_iter = result_stack.rbegin();
					std::advance(result_iter,
						p_first->get_arity());

					ATP_LOGIC_ASSERT(std::distance(result_iter.base(),
						result_stack.end()) == p_first->get_arity());

					// compute our result
					auto func_result = f_func(p_first->get_symbol_id(),
						p_second->get_symbol_id(),
						result_iter.base(), result_stack.end());

					// erase child results from stack
					result_stack.erase(result_iter.base(),
						result_stack.end());

#ifdef ATP_LOGIC_DEFENSIVE
					ATP_LOGIC_ASSERT(result_stack.size() +
						p_first->get_arity() == size_before);
#endif

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
	// statements store references to their creator
	const KnowledgeKernel& m_ker;
	SyntaxNodePtr m_root;
	// cache this
	std::set<size_t> m_free_var_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


