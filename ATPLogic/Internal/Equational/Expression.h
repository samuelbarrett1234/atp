#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the `Expression` class, which is used in the
    `Statement` class. Encapsulates part of a syntax tree without
    an equals sign.

*/


// define this if the Expression class should compute and track its
// height
// #define ATP_LOGIC_EXPR_USE_HEIGHT


#include <memory>
#include <vector>
#include <map>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"
#include "ExprTreeFlyweight.h"
#include "SyntaxNodes.h"
#include "../FreeVarIdSet.h"
#include "../FreeVarMap.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ModelContext;  // forward definition
class Expression;  // forward definition


typedef std::shared_ptr<Expression> ExpressionPtr;


/**
\brief An expression is an encapsulation of the part of a syntax
    tree without an equals sign.

\note As much as this class is supposed to be immutable, in order
    to be usable in standard containers like std::vector, it needs
	a copy constructor and an assignment operator.
*/
class ATP_LOGIC_API Expression
{
public:
    static ExpressionPtr construct(
        const ModelContext& ctx,
        const SyntaxNodePtr& p_root);
	static ExpressionPtr construct(
		Expression&& expr);
	/**
	\brief Create a new object from a binary stream, which must be of
		the correct format.

	\param in The (binary) input stream.
	*/
	static Expression load_from_bin(
		const ModelContext& ctx, std::istream& in);

public:
	/**
	\brief For iterating over all subtrees (subexpressions) of a
		given expression in pre-order traversal
	*/
	class ATP_LOGIC_API iterator
	{
	private:
		struct ATP_LOGIC_API StackFrame
		{
			StackFrame(size_t id, SyntaxNodeType type,
				size_t func_idx, size_t arg_idx, bool is_root) :
				id(id), type(type), func_idx(func_idx),
				arg_idx(arg_idx), is_root(is_root)
			{ }

			bool operator == (const StackFrame& other) const
			{
				return (id == other.id && type == other.type &&
					func_idx == other.func_idx &&
					arg_idx == other.arg_idx &&
					is_root == other.is_root);
			}

			// id/type pair, which is consistent with its usage
			// throughout the rest of the Expression code

			size_t id;
			SyntaxNodeType type;

			// these two help identify the "location" of an element
			// on the stack, and are only needed for non-root
			// elements:
			size_t func_idx, arg_idx;
			bool is_root;
		};

	public:
		// conforming to the standard iterator template interface

		typedef std::forward_iterator_tag iterator_category;
		typedef Expression value_type;
		typedef const Expression& reference;
		typedef const Expression* pointer;
		typedef size_t difference_type;

		inline iterator() : m_parent(nullptr) {}
		inline iterator(const Expression* parent) :
			m_parent(parent)
		{
			ATP_LOGIC_PRECOND(m_parent != nullptr);
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
			const size_t capacity = m_parent->m_height;
#else
			const size_t capacity = m_parent->m_tree.size() + 1;
#endif
			m_stack.reserve(capacity);
			m_stack.emplace_back(m_parent->m_tree.root_id(),
				m_parent->m_tree.root_type(), 0, 0, true);
		}
		inline iterator(const iterator& other) :
			m_stack(other.m_stack),
			m_parent(other.m_parent),
			// see "hack" comment in copy assignment operator
			m_my_value(std::move(other.m_my_value))
		{ }
		inline iterator(iterator&& other) noexcept :
			m_stack(std::move(other.m_stack)),
			m_parent(other.m_parent),
			m_my_value(std::move(other.m_my_value))
		{
			other.m_parent = nullptr;
		}
		inline iterator& operator =(const iterator& other)
		{
			m_stack = other.m_stack;
			m_parent = other.m_parent;
			
			// hack: if it is more probable that we will use the
			// value than them, then because the value is lazily
			// computed, we are permitted to move it across here!
			// this is only bad if "copies don't generally mean we
			// use the value"
			m_my_value = std::move(other.m_my_value);
			// of course, the above value might not even be present

			//m_my_value = std::make_unique<Expression>(
			//	*other.m_my_value);  // warning: won't work if null

			return *this;
		}
		inline iterator& operator =(iterator&& other) noexcept
		{
			m_stack = std::move(other.m_stack);
			m_parent = other.m_parent;
			other.m_parent = nullptr;
			m_my_value = std::move(other.m_my_value);
			return *this;
		}
		inline reference operator*() const
		{
			ATP_LOGIC_PRECOND(m_parent != nullptr);
			ATP_LOGIC_PRECOND(!m_stack.empty());
			return my_value();
		}
		inline pointer operator->() const
		{
			ATP_LOGIC_PRECOND(m_parent != nullptr);
			ATP_LOGIC_PRECOND(!m_stack.empty());
			return &my_value();
		}
		inline iterator& operator++()
		{
			ATP_LOGIC_PRECOND(!is_end_iterator());
			
			if (m_stack.back().type ==
				SyntaxNodeType::FUNC)
			{
				// the ID of a function is just its array index
				const size_t func_idx = m_stack.back().id;
				const size_t arity = m_parent->m_tree.func_arity(
					func_idx);
				
				// remove ourselves from the stack
				m_stack.pop_back();

				// push children onto the stack in *reverse* order
				for (size_t i = 1; i <= arity;
					++i)
				{
					const size_t j = arity - i;

					// note that `func_id` and `j` are enough to
					// compute the location of the child:
					m_stack.emplace_back(
						m_parent->m_tree.func_children(func_idx)[j],
						m_parent->m_tree.func_child_types(
							func_idx)[j],
						func_idx, j,
						/* children are not roots */ false);
				}
			}
			// else we have no children so just remove ourselves from
			// the stack
			else m_stack.pop_back();

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
			return (m_stack == iter.m_stack);
		}
		inline bool operator!=(const iterator& iter) const
		{
			return !(*this == iter);
		}

		// not part of the standard iterator interface

		inline bool is_end_iterator() const
		{
			return m_stack.empty();
		}

		inline bool is_begin_iterator() const
		{
			ATP_LOGIC_PRECOND(m_stack.empty() || m_parent != nullptr);
			const bool is_begin = (!m_stack.empty() && m_stack.back().is_root);

#ifdef ATP_LOGIC_DEFENSIVE
			// if we are a begin iterator then we should also have
			// stack size 1 and the ID/type of the one element on the
			// stack should tie up with m_parent->m_tree's root info.
			ATP_LOGIC_ASSERT(!is_begin ||
				(m_stack.size() == 1 && m_stack.back().id
				== m_parent->m_tree.root_id() && m_stack.back().type
				== m_parent->m_tree.root_type()));
#endif

			return is_begin;
		}

		/**
		\brief Get the function index containing the sub-expression
			pointed to currently by this iterator.

		\pre This iterator isn't pointing to the root, and isn't an
			end iterator.
		*/
		inline size_t parent_func_idx() const
		{
			ATP_LOGIC_PRECOND(!is_end_iterator());
			ATP_LOGIC_PRECOND(!m_stack.back().is_root);
			return m_stack.back().func_idx;
		}

		/**
		\brief Get which argument of the parent function this
			iterator is pointing to.

		\pre This iterator isn't pointing to the root, and isn't
			and end iterator.
		*/
		inline size_t parent_arg_idx() const
		{
			ATP_LOGIC_PRECOND(!is_end_iterator());
			ATP_LOGIC_PRECOND(!m_stack.back().is_root);
			return m_stack.back().arg_idx;
		}

	private:
		/**
		\brief Returns the expression value of this iterator,
			constructing it if necessary
		*/
		inline Expression& my_value() const
		{
			ATP_LOGIC_ASSERT(!is_end_iterator());

			if (m_my_value == nullptr)
			{
				// create sub expression from the parent
				m_my_value = std::make_unique<Expression>(
					m_parent->sub_expression(m_stack.back().id,
						m_stack.back().type));
			}

			return *m_my_value;
		}

	private:
		// if it is empty then we are done
		// the stack represents the nodes left to explore, and the
		// back element is the one we're currently looking at.
		std::vector<StackFrame> m_stack;
		const Expression* m_parent;

		// it is easier to create a copy of the expression here, and
		// just return references and pointers to it
		// mutable because lazily constructed
		mutable std::unique_ptr<Expression> m_my_value;
	};

public:
    /**
    \pre p_root->get_type() != SyntaxNodePtr::EQ
    */
    Expression(const ModelContext& ctx,
        const SyntaxNodePtr& p_root);

	/**
	\brief Construction of very simple (free or constant) expressions

	\pre root_type == FREE or CONSTANT
	*/
	Expression(const ModelContext& ctx,
		size_t root_id, SyntaxNodeType root_type);

    Expression(const Expression& other);
    Expression(Expression&& other) noexcept;
    Expression& operator= (const Expression& other);
    Expression& operator= (Expression&& other) noexcept;

	inline iterator begin() const
	{
		return iterator(this);
	}
	inline iterator end() const
	{
		return iterator();
	}

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	inline size_t height() const
	{
		return m_height;
	}
#endif

	std::string to_str() const;

	const FreeVarIdSet& free_var_ids() const;

	/**
	\brief Get the symbol or free ID of the root of this expression
	*/
	inline size_t root_id() const
	{
		if (m_tree.root_type() != SyntaxNodeType::FUNC)
			return m_tree.root_id();
		else
			return m_tree.func_symb_id(m_tree.root_id());
	}
	inline SyntaxNodeType root_type() const
	{
		return m_tree.root_type();
	}

	// ideally one should only need this when trying to squeeze out
	// as much performance as possible
	inline const ExprTreeFlyweight& tree() const
	{
		return m_tree;
	}

    /**
    \brief Substitute the free variables according to the given map

    \param free_map A mapping from free variable IDs to expressions
        which will replace them

    \pre All free variables must be assigned a mapping (this mapping
        must be total).
    */
    Expression map_free_vars(const FreeVarMap<Expression>& free_map) const;

	/**
	\brief Replace the expression rooted at the given position iter
		with the given new expression

	\returns A new expression containing the result

	\pre `pos` must be an iterator belonging to this class, and must
		not be an end iterator.
	*/
	Expression replace(const iterator& pos,
		const Expression& new_expr) const;

	/**
	\brief Special case of `map_free_vars` for replacing a free
		variable with another free variable.

	\note This function exists separately to `map_free_vars` because
		it can be implemented more efficiently, without a fold.

	\note `initial_id` does not have to be a free variable ID present
		in this expression - and in this case, we just return *this.
	*/
	Expression replace_free_with_free(size_t initial_id,
		size_t after_id) const;

	/**
	\brief Special case of `map_free_vars` for replacing a free
		variable with a user-defined constant.

	\note This function exists separately to `map_free_vars` because
		it can be implemented more efficiently, without a fold.

	\note `initial_id` does not have to be a free variable ID present
		in this expression - and in this case, we just return *this.
	*/
	Expression replace_free_with_const(size_t initial_id,
		size_t const_symb_id) const;

	/**
	\brief Add a particular value to the ID of every free variable in
		this expression.

	\details This is useful for ensuring the free variable IDs of two
		expressions do not clash, for example if we know the largest
		ID in one expression, we can increment the IDs in this
		expression by that amount + 1, then they won't clash.
	*/
	Expression increment_free_var_ids(size_t inc) const;

	/**
	\brief Given an (id, type) pair, return a new expression which
		represents the sub-expression of this parent expression.

	\note If type is just FREE or CONSTANT, this implementation is
		quite trivial and boring. It is only really interesting
		when type is FUNC.

	\pre `type` is not EQ, and if `type` is FUNC then `id` must be a
		valid index, and if `type` is FREE then `id` must be a free
		variable ID in this expression, and if `type` is CONSTANT
		then `id` must be a constant symbol ID in the model context.

	\returns A new expression, which shares as much information as
		possible with the parent.
	*/
	Expression sub_expression(size_t id, SyntaxNodeType type) const;

	/**
	\brief Returns true iff the expressions are equal up to swapping
		of free variable names
	*/
	bool equivalent(const Expression& other) const;

	/**
	\brief Returns true iff the two expressions are identical (i.e.
		same free variable IDs)
	*/
	bool identical(const Expression& other) const;

	/**
	\brief Try to match this expression to the given expression.

	\details This function looks for a substitution of this node's
		free variables which creates a statement identical to `expr`

	\param expr The expression to try matching

	\param p_out_subs An optional output parameter, for extracting
		the matching if successful.

	\returns True if and only if the match was successful.

	\post p_out_subs, if not null, will be given a mapping which is
		total with respect to our free variables.
	*/
	bool try_match(const Expression& expr,
		FreeVarMap<Expression>* p_out_subs) const;

	/**
	\brief Save this object to the given (binary) output

	\param out The (binary) output stream.
	*/
	void save(std::ostream& out) const;

    /**
    \brief Perform a fold operation over this expression.

    \tparam ResultT The return type of this operation.

    \tparam FreeFuncT The function to use as the free variable node
        constructor, with type size_t -> ResultT

    \tparam ConstFuncT The function to use as the constant node
	    constructor, with type size_t -> ResultT

    \tparam FFuncT The function to use as the function node
        constructor, with type size_t x
        std::vector<ResultT>::iterator x
        std::vector<ResultT>::iterator -> ResultT
    */
	template<typename ResultT, typename FreeFuncT,
        typename ConstFuncT, typename FFuncT>
	ResultT fold(FreeFuncT free_func,
		ConstFuncT const_func, FFuncT f_func) const
	{
		// fold from the root
		return fold_from<ResultT>(free_func, const_func, f_func,
			m_tree.root_id(), m_tree.root_type());
	}

    /**
    \brief Perform a fold over pairs of expressions.

    \tparam ResultT The return type (all functions must return this
        type.)

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
        signature `Expression x Expression -> ResultT`, where the
        two arguments are just the two nodes. **NOTE**: you can also
		pass a constant ResultT-convertible value instead, which is
		more efficient.

    \param other The other expression to fold with, which can be
        thought of as appearing on the RHS of the pair fold.
    */
	template<typename ResultT, typename FreePairFuncT,
        typename ConstPairFuncT, typename FuncPairFuncT,
        typename DefaultPairFuncT>
	ResultT fold_pair(FreePairFuncT free_func,
		ConstPairFuncT const_func, FuncPairFuncT f_func,
		DefaultPairFuncT default_func_or_val,
        const Expression& other) const
	{
		// static assertions on the types of the functions given:

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
			std::function<ResultT(Expression, Expression)>>::value
			|| std::is_convertible<DefaultPairFuncT, ResultT>::value,
			"DefaultPairFuncT should be of type (Expression, "
			"Expression) -> ResultT, or should be a constant ResultT.");

		// now proceed with the function:

		std::vector<ResultT> result_stack;
		// stores pairs of IDs or indices:
		std::vector<std::pair<size_t, size_t>> todo_stack;
		std::vector<std::pair<SyntaxNodeType,
			SyntaxNodeType>> todo_stack_types;
		std::vector<bool> seen_stack;

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		// we only go as deep as the shallowest tree
		const size_t capacity = std::min(m_height, other.m_height);
#else
		// we only go as big as the smaller tree (+1 for root)
		const size_t capacity = std::min(m_tree.size(),
			other.m_tree.size()) + 1;
#endif
		// we know approximately how big the stacks will get in
		// advance
		result_stack.reserve(capacity);
		todo_stack.reserve(capacity);
		todo_stack_types.reserve(capacity);
		seen_stack.reserve(capacity);

		// push the left and right of the equals sign
		todo_stack.push_back(std::make_pair(m_tree.root_id(),
			other.m_tree.root_id()));
		todo_stack_types.push_back(std::make_pair(m_tree.root_type(),
			other.m_tree.root_type()));
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

				if constexpr (std::is_convertible<DefaultPairFuncT,
					ResultT>::value)
				{
					// just use the constant value given to us
					result_stack.push_back(default_func_or_val);
				}
				else
				{
					// convert both sides to syntax trees, call as a
					// function, then push the result
					result_stack.emplace_back(default_func_or_val(
						sub_expression(id_pair.first,
							type_pair.first),
						other.sub_expression(id_pair.second,
							type_pair.second)
					));
				}
			}
			else if (!seen)  // else if first occurrence
			{
				switch (type_pair.first)
				{
				case SyntaxNodeType::FREE:
					result_stack.emplace_back(free_func(id_pair.first,
						id_pair.second));
					break;
				case SyntaxNodeType::CONSTANT:
					result_stack.emplace_back(const_func(id_pair.first,
						id_pair.second));
					break;
				case SyntaxNodeType::FUNC:
				{
					// get arity of both functions
					const auto arity_pair = std::make_pair(
						m_tree.func_arity(id_pair.first),
						other.m_tree.func_arity(id_pair.second));

					// use default func if they have differing
					// arities
					if (arity_pair.first != arity_pair.second)
					{
						if constexpr (std::is_convertible<
							DefaultPairFuncT, ResultT>::value)
						{
							// just use the constant value
							result_stack.push_back(
								default_func_or_val);
						}
						else
						{
							// convert both sides to syntax trees,
							// call as a function, then push the
							// result
							result_stack.emplace_back(
								default_func_or_val(
								sub_expression(id_pair.first,
									type_pair.first),
								other.sub_expression(id_pair.second,
									type_pair.second)
							));
						}
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
							m_tree.func_children(id_pair.first);
						const std::array<SyntaxNodeType, MAX_ARITY>&
							child_types_a =
							m_tree.func_child_types(id_pair.first);

						const std::array<size_t, MAX_ARITY>&
							children_b = other.m_tree.func_children(
								id_pair.second);
						const std::array<SyntaxNodeType, MAX_ARITY>&
							child_types_b =
							other.m_tree.func_child_types(
								id_pair.second);

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

				const size_t arity = m_tree.func_arity(id_pair.first);

				ATP_LOGIC_ASSERT(other.m_tree.func_arity(id_pair.second)
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
					m_tree.func_symb_id(id_pair.first),
					other.m_tree.func_symb_id(id_pair.second),
					result_iter.base(),
					result_stack.end());

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

		// should finish with one result corresponding to the root
		ATP_LOGIC_ASSERT(result_stack.size() == 1);

		return result_stack.back();
	}

private:
	/**
	\brief Perform a fold operation over this expression, starting
		from a particular part of the tree

	\tparam ResultT The return type of this operation.

	\tparam FreeFuncT The function to use as the free variable node
		constructor, with type size_t -> ResultT

	\tparam ConstFuncT The function to use as the constant node
		constructor, with type size_t -> ResultT

	\tparam FFuncT The function to use as the function node
		constructor, with type size_t x
		std::vector<ResultT>::iterator x
		std::vector<ResultT>::iterator -> ResultT

	\param start_id The ID or index of the start node (the meaning of
		this depends on `start_type`)

	\param start_type The type of the node associated with the ID or
		index.

	*/
	template<typename ResultT, typename FreeFuncT,
		typename ConstFuncT, typename FFuncT>
	ResultT fold_from(FreeFuncT free_func,
		ConstFuncT const_func, FFuncT f_func,
		size_t start_id, SyntaxNodeType start_type) const
	{
		// static assertions on the types of the functions given:

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

		// we know approximately how big the stacks will get in
		// advance
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		const size_t capacity = m_height;
#else
		const size_t capacity = m_tree.size() + 1;
#endif
		result_stack.reserve(capacity);
		todo_stack.reserve(capacity);
		todo_stack_types.reserve(capacity);
		seen_stack.reserve(capacity);

		// also cache results about constants and free variables
		// (because there will be a large ratio of their occurrences
		// to the number of them there actually are, so we should
		// reuse their results where we can.)
		std::map<size_t, ResultT> const_result_cache;
		FreeVarMap<ResultT> free_var_result_cache;

		// initialise stacks
		todo_stack.push_back(start_id);
		todo_stack_types.push_back(start_type);
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
						free_var_result_cache.insert(id, r);
						result_stack.emplace_back(std::move(r));
					}
					else
					{
						// else reuse old computation
						result_stack.push_back(cache_iter.second());
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
						result_stack.emplace_back(std::move(r));
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

					const size_t arity = m_tree.func_arity(id);
					const std::array<size_t, MAX_ARITY>&
						children = m_tree.func_children(id);
					const std::array<SyntaxNodeType, MAX_ARITY>&
						child_types = m_tree.func_child_types(id);

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

				const size_t arity = m_tree.func_arity(id);

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
				ResultT result = f_func(m_tree.func_symb_id(id),
					result_iter.base(),
					result_stack.end());

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

		// should finish with one result corresponding to the root
		ATP_LOGIC_ASSERT(result_stack.size() == 1);

		return result_stack.back();
	}

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
		SyntaxNodeType> add_tree_data(const SyntaxNodePtr& tree);

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	/**
	\brief Compute the height of this node (stored in `m_height`)

	\details The height of an expression is defined to be the longest
		path from the root to a leaf of the syntax tree of this
		expression, where path length is measured by number of nodes.
		Thus for example, the root by itself would have height 1.
		We care about height because it allows us to estimate stack
		sizes for fold operations (reducing reallocations), and also
		gives us a quick way to tell when two expressions are not
		equivalent.
	*/
	size_t compute_height() const;
#endif

	/**
	\brief Compute m_free_var_ids

	\pre m_tree is constructed properly already and
		!m_free_var_ids.has_value()

	\remark The only reason this is const is because it is used to
		build m_free_var_ids in its (const) getter function. This
		is okay because (i) m_free_var_ids is mutable and (ii) we
		don't modify any other variables anyway.
	*/
	void build_free_var_ids() const;

private:
    // expressions store references to their creator
    const ModelContext& m_ctx;

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	// height of the syntax tree (this is used to optimise folds
	// by knowing how big the stack will need to be in advance)
	size_t m_height;
#endif

	// an efficient encapsulation of the storage of the expression
	// tree
	ExprTreeFlyweight m_tree;

	// mutable because we compute it lazily and we can return it in
	// a const function
	mutable boost::optional<FreeVarIdSet> m_free_var_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


