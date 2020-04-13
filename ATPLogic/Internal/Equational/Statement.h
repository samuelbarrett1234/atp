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
#include "../../Interfaces/IModelContext.h"
#include "SyntaxNodes.h"
#include "Expression.h"


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


class ModelContext;  // forward definition


/**
Implementation of the IStatement interface for equational logic.

\warning This class implements an arity limit on the functions,
    for efficiency.

\note As part of the optimisation, we store a structure almost like
    a table of function nodes, rather than a syntax tree.

\note As much as this class is supposed to be immutable, in order
    to be usable in standard containers like std::vector, it needs
	a copy constructor and an assignment operator.

\note Statement objects are tied to a model context, but not to any
	particular knowledge kernel.
*/
class ATP_LOGIC_API Statement : public IStatement
{
public:
	/**
	\pre p_root->get_type() == SyntaxNodeType::EQ
	*/
	Statement(const ModelContext& ctx,
		const SyntaxNodePtr& p_root);
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

	inline const ModelContext& context() const
	{
		return m_ctx;
	}

	/**
	\brief Special case of `map_free_vars` for replacing a free
		variable with another free variable.

	\note This function exists separately to `map_free_vars` because
		it can be implemented more efficiently, without a fold.
	*/
	Statement replace_free_with_free(size_t initial_id,
		size_t after_id) const;

	/**
	\brief Special case of `map_free_vars` for replacing a free
		variable with a user-defined constant.

	\note This function exists separately to `map_free_vars` because
		it can be implemented more efficiently, without a fold.
	*/
	Statement replace_free_with_const(size_t initial_id,
		size_t const_symb_id) const;

	/**
	\brief Add a particular value to the ID of every free variable in
		this statement.

	\details This is useful for ensuring the free variable IDs of two
		statements do not clash, for example if we know the largest
		ID in one statement, we can increment the IDs in this
		statement by that amount + 1, then they won't clash.
	*/
	Statement increment_free_var_ids(size_t inc) const;


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
		// only check type of the Eq function, because the other
		// functions will be type-checked when we delegate to
		// Expression::fold
		static_assert(std::is_convertible<EqFuncT,
			std::function<ResultT(ResultT, ResultT)>>::value,
			"EqFuncT should be of type (ResultT, ResultT) -> ResultT");

		ResultT left_result = m_sides.first->fold<ResultT>(
			free_func, const_func, f_func);

		ResultT right_result = m_sides.second->fold<ResultT>(
			free_func, const_func, f_func);

		return eq_func(std::move(left_result),
			std::move(right_result));
	}


	/**
	\brief Perform a fold over pairs of statements, where the fold
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
		// only check type of the Eq function, because the other
		// functions will be type-checked when we delegate to
		// Expression::fold_pair
		static_assert(std::is_convertible<EqPairFuncT,
			std::function<ResultT(ResultT, ResultT)>>::value,
			"EqPairFuncT should be of type (ResultT, ResultT) -> ResultT");

		ResultT left_result = m_sides.first->fold_pair<ResultT>(
			free_func, const_func, f_func, default_func,
			*other.m_sides.first);

		ResultT right_result = m_sides.second->fold_pair<ResultT>(
			free_func, const_func, f_func, default_func,
			*other.m_sides.second);

		return eq_func(std::move(left_result),
			std::move(right_result));
	}


private:
	// statements store references to their creator
	const ModelContext& m_ctx;

	// a statement is just a pair of expressions, the first of which
	// representing the LHS and the second representing the RHS,
	// appearing on each side of the equals sign.
	std::pair<ExpressionPtr, ExpressionPtr> m_sides;

	// cache the sides of this statement in a tree representation
	// for efficiency, because `get_sides` is called a lot.
	std::pair<SyntaxNodePtr, SyntaxNodePtr> m_tree_sides;

	std::set<size_t> m_free_var_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


