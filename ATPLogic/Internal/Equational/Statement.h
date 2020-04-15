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
	\brief For iterating over all subtrees (subexpressions) of a
		given statement in pre-order traversal, starting with the
		left-hand expression and ending with the right-hand one.

	\details A statement iterator is just an iterator for the LHS
		and an iterator for the RHS, which then uses the LHS iterator
		until it becomes invalid, and when that happens it
		subsequently just uses the RHS iterator until that becomes
		invalid too.
	*/
	class ATP_LOGIC_API iterator
	{
		friend Statement;
	public:
		// conforming to the standard iterator template interface

		typedef std::forward_iterator_tag iterator_category;
		typedef Expression value_type;
		typedef const Expression& reference;
		typedef const Expression* pointer;
		typedef size_t difference_type;

		inline iterator() = default;
		inline iterator(const Statement& parent) :
			m_left(parent.m_sides.first->begin()),
			m_right(parent.m_sides.second->begin())
		{
		}
		inline iterator(const iterator& other) :
			m_left(other.m_left),
			m_right(other.m_right)
		{ }
		inline iterator(iterator&& other) noexcept :
			m_left(std::move(other.m_left)),
			m_right(std::move(other.m_right))
		{
		}
		inline iterator& operator =(const iterator& other)
		{
			m_left = other.m_left;
			m_right = other.m_right;
			return *this;
		}
		inline iterator& operator =(iterator&& other) noexcept
		{
			m_left = std::move(other.m_left);
			m_right = std::move(other.m_right);
			return *this;
		}
		inline reference operator*() const
		{
			if (!m_left.is_end_iterator())
				return *m_left;
			else
				return *m_right;
		}
		inline pointer operator->() const
		{
			if (!m_left.is_end_iterator())
				return m_left.operator->();
			else
				return m_right.operator->();
		}
		inline iterator& operator++()
		{
			if (!m_left.is_end_iterator())
				++m_left;
			else
				++m_right;
			return *this;
		}
		inline iterator operator++(int)
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}
		inline bool operator==(const iterator& iter) const
		{
			return (m_left == iter.m_left &&
				m_right == iter.m_right);
		}
		inline bool operator!=(const iterator& iter) const
		{
			return !(*this == iter);
		}

	private:
		Expression::iterator m_left, m_right;
	};

public:
	/**
	\pre p_root->get_type() == SyntaxNodeType::EQ
	*/
	Statement(const ModelContext& ctx,
		const SyntaxNodePtr& p_root);
	Statement(const ModelContext& ctx,
		Expression lhs, Expression rhs);
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

	inline iterator begin() const
	{
		return iterator(*this);
	}
	inline iterator end() const
	{
		return iterator();
	}

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

	inline Expression lhs() const
	{
		return *m_sides.first;
	}
	inline Expression rhs() const
	{
		return *m_sides.second;
	}

	/**
	\brief Substitute the free variables according to the given map

	\param free_map A mapping from free variable IDs to expressions
		which will replace them

	\pre All free variables must be assigned a mapping (this mapping
		must be total).
	*/
	Statement map_free_vars(const std::map<size_t,
		Expression> free_map) const;

	/**
	\brief Replace the expression rooted at the given position iter
		with the given new expression

	\returns A new statement containing the result

	\pre `pos` must be an iterator belonging to this class, and must
		not be an end iterator.
	*/
	Statement replace(const iterator& pos,
		const Expression& expr) const;

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
	\brief Transpose the statement (flip it around the equals sign)

	\details This can be implemented using a fold, but doing so is
		very slow as it requires reallocating and rebuilding the
		whole tree.
	*/
	Statement transpose() const;

	/**
	\brief Returns true iff the statements are equal up to swapping
		of free variable names and reflection about the equals sign
	*/
	bool equivalent(const Statement& other) const;

	/**
	\brief Returns true iff the two statements are identical (i.e.
		same free variable IDs, same orientation about equals sign,
		etc.)
	*/
	inline bool identical(const Statement& other) const
	{
		return m_sides.first->identical(*other.m_sides.first)
			&& m_sides.second->identical(*other.m_sides.second);
	}

	/**
	\brief Returns true iff the statement is symmetric about the
		equals sign (thus trivially true by reflexivity of '=').
	*/
	inline bool true_by_reflexivity() const
	{
		return m_sides.first->identical(*m_sides.second);
	}

	/**
	\brief Returns true iff there exists a substitution in the
		premise (the callee) which would produce the conclusion

	\warning This is not used in finding proofs, because in this
		implementation of equational logic, proofs are "iff", i.e.
		readable in both directions.
	*/
	bool implies(const Statement& conclusion) const;

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
	/**
	\brief Recompute m_free_var_ids after it may have been outdated

	\pre m_sides is constructed properly already
	*/
	void rebuild_free_var_ids();

private:
	// statements store references to their creator
	const ModelContext& m_ctx;

	// a statement is just a pair of expressions, the first of which
	// representing the LHS and the second representing the RHS,
	// appearing on each side of the equals sign.
	std::pair<ExpressionPtr, ExpressionPtr> m_sides;

	std::set<size_t> m_free_var_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


