#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the implementation of the IStatement interface for
    equational logic.

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
#include <array>
#include <map>
#include <set>
#include <boost/iterator/zip_iterator.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "SyntaxNodes.h"


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

\warning This class implements an arity limit on the functions,
    for efficiency.

\note As part of the optimisation, we store a structure almost like
    a table of function nodes, rather than a syntax tree.

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

	// the only reason this is here is so that we can put Statement
	// objects into a vector... try to avoid using this otherwise!
	Statement& operator= (Statement&& other) noexcept;

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

	/**
	\brief Substitute the free variables according to the given map

	\param free_map A mapping from free variable IDs to expressions
	    which will replace them

	\pre All free variables must be assigned a mapping (this mapping
	    must be total).
	*/
	Statement map_free_vars(const std::map<size_t,
		SyntaxNodePtr> free_map) const;

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
				typename std::vector<ResultT>::iterator,
				typename std::vector<ResultT>::iterator)>>::value,
			"FFuncT should be of type (size_t, "
			"std::vector<ResultT>::iterator,"
			" std::vector<ResultT>::iterator) -> ResultT");

		// now proceed with the function:

		std::vector<ResultT> result_stack;
		std::vector<size_t> todo_stack;  // stores IDs or indices
		std::vector<SyntaxNodeType> todo_stack_types;
		std::vector<bool> seen_stack;

		// also cache results about constants and free variables
		// (because there will be a large ratio of their occurrences
		// to the number of them there actually are, so we should
		// reuse their results where we can.)
		std::map<size_t, ResultT> free_var_result_cache,
			const_result_cache;
		
		// push the left and right of the equals sign
		todo_stack.push_back(m_left);
		todo_stack_types.push_back(m_left_type);
		seen_stack.push_back(false);
		todo_stack.push_back(m_right);
		todo_stack_types.push_back(m_right_type);
		seen_stack.push_back(false);

		while (!todo_stack.empty())
		{
			// check stack size invariants

			ATP_LOGIC_ASSERT(todo_stack.size() ==
				todo_stack_types.size());
			ATP_LOGIC_ASSERT(todo_stack.size() ==
				seen_stack.size());

			const size_t id = todo_stack.back();
			todo_stack.pop_back();
			const SyntaxNodeType type = todo_stack_types.back();
			todo_stack_types.pop_back();
			const bool seen = seen_stack.back();
			seen_stack.pop_back();

			// if first occurrence
			if (!seen)
			{
				switch (type)
				{
				case SyntaxNodeType::FREE:
				{
					// look for already computed value in the cache
					auto cache_iter = free_var_result_cache.find(id);

					// a new result to compute
					if (cache_iter == free_var_result_cache.end())
					{
						ResultT r = free_func(id);
						free_var_result_cache[id] = r;
						result_stack.push_back(r);
					}
					else
					{
						// else reuse old computation
						result_stack.push_back(cache_iter->second);
					}
				}
					break;
				case SyntaxNodeType::CONSTANT:
				{
					// look for already computed value in the cache
					auto cache_iter = const_result_cache.find(id);

					// a new result to compute
					if (cache_iter == const_result_cache.end())
					{
						ResultT r = const_func(id);
						const_result_cache[id] = r;
						result_stack.push_back(r);
					}
					else
					{
						// else reuse old computation
						result_stack.push_back(cache_iter->second);
					}
				}
				break;
				case SyntaxNodeType::FUNC:
				{
					// firstly, add us back to the stack for viewing
					// again later once our child nodes have been
					// visited:

					todo_stack.push_back(id);
					todo_stack_types.push_back(type);
					seen_stack.push_back(true);

					// get some info about us:

					const size_t arity = m_func_arity[id];
					const std::array<size_t, MAX_ARITY>&
						children = m_func_children[id];
					const std::array<SyntaxNodeType, MAX_ARITY>&
						child_types = m_func_child_types[id];

#ifdef ATP_LOGIC_DEFENSIVE
					const size_t todo_size_before
						= todo_stack.size();
#endif

					// now add children (in reverse order!)
					// (warning: don't forget that those arrays
					// aren't necessarily full!)
					todo_stack.insert(todo_stack.end(),
						children.rbegin() + (MAX_ARITY - arity),
						children.rend());
					todo_stack_types.insert(todo_stack_types.end(),
						child_types.rbegin() + (MAX_ARITY - arity),
						child_types.rend());
					seen_stack.resize(seen_stack.size() + arity,
						false);

#ifdef ATP_LOGIC_DEFENSIVE
					ATP_LOGIC_ASSERT(todo_stack.size() ==
						todo_size_before + arity);
#endif
				}
				break;
				default:
					ATP_LOGIC_ASSERT("invalid type detected during"
						"fold!" && false);
				}
			}
			else
			{
				// only functions get put into the stack twice
				ATP_LOGIC_ASSERT(type == SyntaxNodeType::FUNC);

				const size_t arity = m_func_arity[id];

				ATP_LOGIC_ASSERT(result_stack.size() >= arity);

#ifdef ATP_LOGIC_DEFENSIVE
				const size_t size_before = result_stack.size();
#endif

				// find list of child results:
				auto result_iter = result_stack.rbegin();
				std::advance(result_iter, arity);

				ATP_LOGIC_ASSERT(std::distance(result_iter.base(),
					result_stack.end()) == arity);

				// now compute result for p_func:
				ResultT result = f_func(m_func_symb_ids[id],
					result_iter.base(),
					result_stack.end());

				// now remove the child results and add ours:
				result_stack.erase(result_iter.base(),
					result_stack.end());

#ifdef ATP_LOGIC_DEFENSIVE
				ATP_LOGIC_ASSERT(result_stack.size() + arity
					== size_before);
#endif

				result_stack.push_back(result);
			}
		}

		// handle the Eq node at the end (we should have results
		// for the left and right).
		ATP_LOGIC_ASSERT(result_stack.size() == 2);

		ResultT left_result = result_stack.back();
		result_stack.pop_back();
		ResultT right_result = result_stack.back();
		result_stack.pop_back();

		// this function should not be needed elsewhere
		// except for right at the end!
		return eq_func(left_result, right_result);
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
	    nodes, must have signature `size_t x size_t x std::vector<
		ResultT>::iterator x std::vector<ResultT>::iterator -> ResultT`
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
			"EqPairFuncT should be of type (ResultT, ResultT) -> ResultT");
		static_assert(std::is_convertible<FreePairFuncT,
			std::function<ResultT(size_t, size_t)>>::value,
			"FreePairFuncT should be of type (size_t, size_t) -> ResultT");
		static_assert(std::is_convertible<ConstPairFuncT,
			std::function<ResultT(size_t, size_t)>>::value,
			"ConstPairFuncT should be of type (size_t, size_t) -> ResultT");
		static_assert(std::is_convertible<FuncPairFuncT,
			std::function<ResultT(size_t, size_t,
				typename std::vector<ResultT>::iterator,
				typename std::vector<ResultT>::iterator)>>::value,
			"FuncPairFuncT should be of type (size_t, size_t, "
			"std::list<ResultT>::iterator,"
			" std::list<ResultT>::iterator) -> ResultT");
		static_assert(std::is_convertible<DefaultPairFuncT,
			std::function<ResultT(SyntaxNodePtr, SyntaxNodePtr)>>::value,
			"DefaultPairFuncT should be of type (SyntaxNodePtr, "
			"SyntaxNodePtr) -> ResultT");

		// now proceed with the function:

		std::vector<ResultT> result_stack;
		// stores pairs of IDs or indices:
		std::vector<std::pair<size_t, size_t>> todo_stack;
		std::vector<std::pair<SyntaxNodeType,
			SyntaxNodeType>> todo_stack_types;
		std::vector<bool> seen_stack;

		// push the left and right of the equals sign
		todo_stack.push_back(std::make_pair(m_left,
			other.m_left));
		todo_stack_types.push_back(std::make_pair(m_left_type,
			other.m_left_type));
		seen_stack.push_back(false);
		todo_stack.push_back(std::make_pair(m_right,
			other.m_right));
		todo_stack_types.push_back(std::make_pair(m_right_type,
			other.m_right_type));
		seen_stack.push_back(false);

		while (!todo_stack.empty())
		{
			// check stack size invariants

			ATP_LOGIC_ASSERT(todo_stack.size() ==
				todo_stack_types.size());
			ATP_LOGIC_ASSERT(todo_stack.size() ==
				seen_stack.size());

			const std::pair<size_t,
				size_t> id_pair = todo_stack.back();
			todo_stack.pop_back();
			const std::pair<SyntaxNodeType,
				SyntaxNodeType> type_pair = todo_stack_types.back();
			todo_stack_types.pop_back();
			const bool seen = seen_stack.back();
			seen_stack.pop_back();

			if (type_pair.first != type_pair.second)
			{
				ATP_LOGIC_ASSERT(!seen);

				// convert both sides to syntax trees and then
				// push the result
				result_stack.push_back(default_func(
					to_syntax_tree(id_pair.first,
						type_pair.first),
					other.to_syntax_tree(id_pair.second,
						type_pair.second)
				));
			}
			else if (!seen)  // else if first occurrence
			{
				switch (type_pair.first)
				{
				case SyntaxNodeType::FREE:
					result_stack.push_back(free_func(id_pair.first,
						id_pair.second));
					break;
				case SyntaxNodeType::CONSTANT:
					result_stack.push_back(const_func(id_pair.first,
						id_pair.second));
					break;
				case SyntaxNodeType::FUNC:
				{
					// get arity of both functions
					const auto arity_pair = std::make_pair(
						m_func_arity[id_pair.first],
						other.m_func_arity[id_pair.second]);

					// use default func if they have differing
					// arities
					if (arity_pair.first != arity_pair.second)
					{
						result_stack.push_back(default_func(
							to_syntax_tree(id_pair.first,
								type_pair.first),
							other.to_syntax_tree(id_pair.second,
								type_pair.second)
						));
					}
					else
					{

						// firstly, add us back to the stack for
						// viewing again later once our child nodes
						// have been visited:

						todo_stack.push_back(id_pair);
						todo_stack_types.push_back(type_pair);
						seen_stack.push_back(true);

						// get some info about us:

						const auto arity = arity_pair.first;
						ATP_LOGIC_ASSERT(arity_pair.first ==
							arity_pair.second);
						
						const std::array<size_t, MAX_ARITY>&
							children_a =
							m_func_children[id_pair.first];
						const std::array<SyntaxNodeType, MAX_ARITY>&
							child_types_a =
							m_func_child_types[id_pair.first];

						const std::array<size_t, MAX_ARITY>&
							children_b = 
							other.m_func_children[id_pair.second];
						const std::array<SyntaxNodeType, MAX_ARITY>&
							child_types_b = 
							other.m_func_child_types[id_pair.second];

#ifdef ATP_LOGIC_DEFENSIVE
						const size_t todo_size_before
							= todo_stack.size();
#endif

						// now add children (in reverse order!)
						// (warning: don't forget that those arrays
						// aren't necessarily full!)

						std::transform(boost::make_zip_iterator(
							boost::make_tuple(
								children_a.rbegin()
								+ (MAX_ARITY - arity),
								children_b.rbegin()
								+ (MAX_ARITY - arity))),
							boost::make_zip_iterator(
								boost::make_tuple(
								children_a.rend(),
									children_b.rend())),
							std::back_inserter(todo_stack),
							[](boost::tuple<size_t,
								size_t> tup)
							{ return std::make_pair(tup.get<0>(),
								tup.get<1>()); });

						std::transform(boost::make_zip_iterator(
							boost::make_tuple(
								child_types_a.rbegin()
								+ (MAX_ARITY - arity),
								child_types_b.rbegin()
								+ (MAX_ARITY - arity))),
							boost::make_zip_iterator(
								boost::make_tuple(
									child_types_a.rend(),
									child_types_b.rend())),
							std::back_inserter(todo_stack_types),
							[](boost::tuple<SyntaxNodeType,
								SyntaxNodeType> tup)
							{ return std::make_pair(tup.get<0>(),
								tup.get<1>()); });

						seen_stack.resize(seen_stack.size() + arity,
							false);

#ifdef ATP_LOGIC_DEFENSIVE
						ATP_LOGIC_ASSERT(todo_stack.size() ==
							todo_size_before + arity);
#endif
					}
				}
				break;
				default:
					ATP_LOGIC_ASSERT("invalid type detected during"
						"fold!" && false);
				}
			}
			else
			{
				// only functions get put into the stack twice
				ATP_LOGIC_ASSERT(type_pair.first
					== SyntaxNodeType::FUNC);

				const size_t arity = m_func_arity[id_pair.first];

				ATP_LOGIC_ASSERT(other.m_func_arity[id_pair.second]
					== arity);

				ATP_LOGIC_ASSERT(result_stack.size() >= arity);

#ifdef ATP_LOGIC_DEFENSIVE
				const size_t size_before = result_stack.size();
#endif

				// find list of child results:
				auto result_iter = result_stack.rbegin();
				std::advance(result_iter, arity);

				ATP_LOGIC_ASSERT(std::distance(result_iter.base(),
					result_stack.end()) == arity);

				// now compute result for p_func:
				ResultT result = f_func(
					m_func_symb_ids[id_pair.first],
					other.m_func_symb_ids[id_pair.second],
					result_iter.base(),
					result_stack.end());

				// now remove the child results and add ours:
				result_stack.erase(result_iter.base(),
					result_stack.end());

#ifdef ATP_LOGIC_DEFENSIVE
				ATP_LOGIC_ASSERT(result_stack.size() + arity
					== size_before);
#endif

				result_stack.push_back(result);
			}
		}

		// handle the Eq node at the end (we should have results
		// for the left and right).
		ATP_LOGIC_ASSERT(result_stack.size() == 2);

		ResultT left_result = result_stack.back();
		result_stack.pop_back();
		ResultT right_result = result_stack.back();
		result_stack.pop_back();

		// this function should not be needed elsewhere
		// except for right at the end!
		return eq_func(left_result, right_result);
	}

private:
	/**
	\brief Convert something into a syntax tree

	\details If `type` is free or constant, then we just use `idx` as
	    the free var ID or constant symbol (resp.) and return a
		syntax node representing that. If type == FUNC, then instead
		we recursively construct a subtree rooted at that function
		index our stored table.

	\pre Type is: FREE, CONSTANT, FUNC (but \b not EQ).
	*/
	SyntaxNodePtr to_syntax_tree(size_t idx,
		SyntaxNodeType type) const;

	/**
	\brief Fold the given tree, adding any functions to our function
	    table as we go.

	\details This is useful for adding functions from a tree into
	    this object's state.

	\pre This tree contains no Eq nodes.

	\returns (ID, Type) pair of the root node, in accordance with the
	    usage of such a pair in the rest of this class.
	*/
	std::pair<size_t,
		SyntaxNodeType> add_tree_data(SyntaxNodePtr tree);

private:
	// statements store references to their creator
	const KnowledgeKernel& m_ker;

	/**
	\warning we impose a function arity limit for efficiency!
	*/
	static const constexpr size_t MAX_ARITY = 5;

	// the left and right of the equals sign
	size_t m_left, m_right;
	SyntaxNodeType m_left_type, m_right_type;

	// m_func_symb_ids[i] is the symbol ID of the ith function node
	std::vector<size_t> m_func_symb_ids;

	// m_func_arity[i] is the arity of the ith function
	std::vector<size_t> m_func_arity;

	// m_func_children[i] is the array of size `m_func_arity[i]` of
	// children of that function, and m_func_child_types[i] the
	// corresponding types.
	// if m_func_child_types[i][j] == SyntaxNodeType::FREE,
	// then m_func_children[i][j] is the free variable ID.
	// if m_func_child_types[i][j] == SyntaxNodeType::CONSTANT,
	// then m_func_children[i][j] is the constant symbol ID.
	// if m_func_child_types[i][j] == SyntaxNodeType::FUNC,
	// then m_func_children[i][j] is the index of the function in
	// these arrays.

	std::vector<std::array<size_t, MAX_ARITY>> m_func_children;
	std::vector<std::array<SyntaxNodeType, MAX_ARITY>> m_func_child_types;

	// cache these
	std::set<size_t> m_free_var_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


