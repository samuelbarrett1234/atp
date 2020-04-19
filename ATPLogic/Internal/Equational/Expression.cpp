/**

\file

\author Samuel Barrett

*/


#include "Expression.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include <boost/bimap.hpp>
#include "SyntaxTreeFold.h"
#include "ModelContext.h"


namespace phx = boost::phoenix;
namespace phxarg = boost::phoenix::arg_names;


namespace atp
{
namespace logic
{
namespace equational
{


ExpressionPtr Expression::construct(const ModelContext& ctx,
	const SyntaxNodePtr& p_root)
{
	return std::make_shared<Expression>(ctx, p_root);
}


ExpressionPtr Expression::construct(Expression&& expr)
{
	return std::make_shared<Expression>(std::move(expr));
}


Expression::Expression(const ModelContext& ctx,
	size_t root_id, SyntaxNodeType root_type) :
	m_ctx(ctx)
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	, m_height(1)
#endif
{
	ATP_LOGIC_PRECOND(root_type != SyntaxNodeType::FUNC);
	ATP_LOGIC_PRECOND(root_type !=
		SyntaxNodeType::EQ);  // this one is obvious

	m_tree.set_root(root_id, root_type);
}


Expression::Expression(const ModelContext& ctx,
	const SyntaxNodePtr& p_root) :
	m_ctx(ctx)
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	, m_height(0)
#endif
{
	ATP_LOGIC_PRECOND(p_root->get_type() !=
		SyntaxNodeType::EQ);

	auto root_data = add_tree_data(p_root);

	// it is important we set this, or `m_tree` will be left in an
	// invalid state
	m_tree.set_root(root_data.first, root_data.second);

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	m_height = compute_height();
#endif
}


Expression::Expression(const Expression& other) :
	m_ctx(other.m_ctx),
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	m_height(other.m_height),
#endif
	m_tree(other.m_tree),
	m_free_var_ids(other.m_free_var_ids)
{ }


Expression::Expression(Expression&& other) noexcept :
	m_ctx(other.m_ctx),
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	m_height(other.m_height),
#endif
	m_tree(std::move(other.m_tree)),
	m_free_var_ids(std::move(other.m_free_var_ids))
{ }


Expression& Expression::operator=(const Expression& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		m_height = other.m_height;
#endif
		m_tree = other.m_tree;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Expression& Expression::operator=(Expression&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		m_height = other.m_height;
#endif
		m_tree = std::move(other.m_tree);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


std::string Expression::to_str() const
{
	// this is a fold!

	auto free_var_fold = [](size_t free_var_id) -> std::string
	{
		return "x" + boost::lexical_cast<std::string>(free_var_id);
	};
	auto const_fold = [this](size_t symb_id) -> std::string
	{
		return m_ctx.symbol_name(symb_id);
	};
	auto func_fold = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return m_ctx.symbol_name(symb_id) + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ") + ')';
	};
	return fold<std::string>(free_var_fold, const_fold, func_fold);
}


const FreeVarIdSet& Expression::free_var_ids() const
{
	if (!m_free_var_ids.has_value())
		build_free_var_ids();

	return m_free_var_ids.get();
}


Expression Expression::map_free_vars(const FreeVarMap<
	Expression>& free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(free_var_ids().begin(),
		free_var_ids().end(),
		[&free_map](size_t id)
		{ return free_map.contains(id); }));

	switch (m_tree.root_type())
	{
	case SyntaxNodeType::CONSTANT:
		return *this;  // unchanged
	case SyntaxNodeType::FREE:
		return free_map.at(m_tree.root_id());
	case SyntaxNodeType::FUNC:
	{
		// strategy: concatenate all of the arrays in the free
		// map expressions by merging them into `new_expr`

		// note that this merge operation will change what we
		// want the substitutions to point to, thus create a
		// `new_map` which maps the root IDs of the `free_map`
		// to the root IDs which should set in `new_expr`.

		// finally, looping through the portion of the function
		// arrays in `new_expr` that were present BEFORE the
		// concatenation, we need to update all free variables in
		// that with their substitutions.

		Expression new_expr = *this;
		new_expr.m_free_var_ids = boost::none;

		const size_t size_before = new_expr.m_tree.size();

		std::map<size_t, size_t> new_map;
		for (auto iter = free_map.begin(); iter != free_map.end();
			++iter)
		{
			new_map[iter.first()] = new_expr.m_tree.merge_from(
				iter.second().m_tree);
		}
		
		for (size_t i = 0; i < size_before; ++i)
		{
			const size_t arity = new_expr.m_tree.func_arity(i);
			const auto& children = new_expr.m_tree.func_children(i);
			const auto& child_types =
				new_expr.m_tree.func_child_types(i);
			for (size_t j = 0; j < arity; ++j)
			{
				if (child_types[j] == SyntaxNodeType::FREE)
				{
					// will need to substitute
					auto id_iter = new_map.find(children[j]);
					auto expr_iter = free_map.find(children[j]);

					ATP_LOGIC_ASSERT(id_iter != new_map.end());
					ATP_LOGIC_ASSERT(expr_iter != free_map.end());

					// note that sub_id is the `new_map` transformation
					// of expr_iter->second.m_tree.root_id()
					auto sub_id = id_iter->second;
					auto sub_type = expr_iter.second().m_tree.root_type();

					new_expr.m_tree.update_func_child(i, j,
						sub_id, sub_type);
				}
			}
		}

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		new_expr.m_height = new_expr.compute_height();
#endif

		return new_expr;
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
		throw std::exception();
	}
}


Expression Expression::replace(const iterator& pos,
	const Expression& sub_expr) const
{
	ATP_LOGIC_PRECOND(!pos.is_end_iterator());

	// handle root position as a special case:
	if (pos.is_begin_iterator())
	{
		// check that, if it is a begin iterator, it at least agrees
		// with our root id/type
		ATP_LOGIC_PRECOND(pos->m_tree.root_id() ==
			m_tree.root_id());
		ATP_LOGIC_PRECOND(pos->m_tree.root_type() ==
			m_tree.root_type());

		// if we really wanted to be pedantic we could also check
		// that pos->m_tree == m_tree, however we don't have an
		// equality operator for m_tree.

		return sub_expr;
	}
	else
	{
		// check this iterator is referencing a valid location
		ATP_LOGIC_PRECOND(pos.parent_func_idx() < m_tree.size());
		ATP_LOGIC_PRECOND(pos.parent_arg_idx() <
			m_tree.func_arity(pos.parent_func_idx()));

		Expression result_expr = *this;
		result_expr.m_free_var_ids = boost::none;

		switch (sub_expr.m_tree.root_type())
		{
		case SyntaxNodeType::FREE:
		case SyntaxNodeType::CONSTANT:
			// for frees and constants it's as easy as just directly
			// substituting in the free/constant ID into a duplicate
			// expression.

			result_expr.m_tree.update_func_child(
				pos.parent_func_idx(), pos.parent_arg_idx(),
				sub_expr.m_tree.root_id(), sub_expr.m_tree.root_type());

			break;
		case SyntaxNodeType::FUNC:
		{
			// for functions, we need to concatenate the arrays of
			// `sub_expr` and `result_expr`, and then update the
			// information at the `pos` iterator similarly to how
			// we did so for frees and constants

			const size_t new_root_id = result_expr.m_tree.merge_from(
				sub_expr.m_tree);

			// warning: we can't use `sub_expr.m_tree.root_id()`
			// because that may not be correct anymore

			result_expr.m_tree.update_func_child(
				pos.parent_func_idx(), pos.parent_arg_idx(),
				new_root_id, sub_expr.m_tree.root_type());

			break;
		}
		default:
			ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
			throw std::exception();
		}

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		// update height value
		result_expr.m_height = result_expr.compute_height();
#endif

		return result_expr;
	}
}


Expression Expression::replace_free_with_free(
	size_t initial_id, size_t after_id) const
{
	// handle the special case where `initial_id` is not an ID
	// present in this expression (this is desired behaviour
	// so `Statement` can replace its free variables by just
	// delegating the call to the LHS and RHS expressions - if we
	// enforced `initial_id` being present in this expression then
	// such an operation would be more cumbersome to implement).
	if (!free_var_ids().contains(initial_id))
		return *this;

	Expression new_expr = *this;
	new_expr.m_free_var_ids = boost::none;

	for (size_t i = 0; i < new_expr.m_tree.size(); ++i)
	{
		const size_t arity = new_expr.m_tree.func_arity(i);
		const auto& children = new_expr.m_tree.func_children(i);
		const auto& child_types = new_expr.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE &&
				children[j] == initial_id)
			{
				new_expr.m_tree.update_func_child(i, j, after_id,
					SyntaxNodeType::FREE);
			}
		}
	}

	return new_expr;
}


Expression Expression::replace_free_with_const(
	size_t initial_id, size_t const_symb_id) const
{
	// handle the special case where `initial_id` is not an ID
	// present in this expression (this is desired behaviour
	// so `Statement` can replace its free variables by just
	// delegating the call to the LHS and RHS expressions - if we
	// enforced `initial_id` being present in this expression then
	// such an operation would be more cumbersome to implement).
	if (!free_var_ids().contains(initial_id))
		return *this;

#ifdef ATP_LOGIC_DEFENSIVE
	// check the constant symbol ID is valid
	const auto symb_ids = m_ctx.all_constant_symbol_ids();
	ATP_LOGIC_PRECOND((std::find(symb_ids.begin(),
		symb_ids.end(), const_symb_id) != symb_ids.end()));
#endif

	Expression new_expr = *this;
	new_expr.m_free_var_ids = boost::none;

	for (size_t i = 0; i < new_expr.m_tree.size(); ++i)
	{
		const size_t arity = new_expr.m_tree.func_arity(i);
		const auto& children = new_expr.m_tree.func_children(i);
		const auto& child_types = new_expr.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE &&
				children[j] == initial_id)
			{
				new_expr.m_tree.update_func_child(i, j, const_symb_id,
					SyntaxNodeType::CONSTANT);
			}
		}
	}

	return new_expr;
}


Expression Expression::increment_free_var_ids(size_t inc) const
{
	// handle this special case more efficiently:
	if (inc == 0)
		return *this;

	Expression new_expr = *this;
	new_expr.m_free_var_ids = boost::none;

	for (size_t i = 0; i < new_expr.m_tree.size(); ++i)
	{
		const size_t arity = new_expr.m_tree.func_arity(i);
		const auto& children = new_expr.m_tree.func_children(i);
		const auto& child_types = new_expr.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE)
			{
				new_expr.m_tree.update_func_child(i, j,
					children[j] + inc, SyntaxNodeType::FREE);
			}
		}
	}

	// handle root as special case
	if (new_expr.m_tree.root_type() == SyntaxNodeType::FREE)
	{
		new_expr.m_tree.set_root(new_expr.m_tree.root_id() + inc,
			SyntaxNodeType::FREE);
	}

	return new_expr;
}


Expression Expression::sub_expression(size_t id, SyntaxNodeType type) const
{
	switch (type)
	{
	case SyntaxNodeType::FREE:
	case SyntaxNodeType::CONSTANT:
		return Expression(m_ctx, id, type);
	case SyntaxNodeType::FUNC:
	{
		ATP_LOGIC_PRECOND(id < m_tree.size());

		Expression new_expr = *this;
		new_expr.m_free_var_ids = boost::none;
		new_expr.m_tree.set_root(id, type);

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
		// update height value
		new_expr.m_height = new_expr.compute_height();
#endif

		return new_expr;
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type");
		throw std::exception();
	}
}


bool Expression::equivalent(const Expression& other) const
{
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	// if two expressions have different heights then they are not
	// equivalent:
	if (m_height != other.m_height)
		return false;
#endif

	// this function is absolutely an instance of a fold, but we can
	// optimise it beyond the generic fold implementation (and this
	// is important because this function is called a lot)
	// the optimisation is just because we don't need the "result
	// stack" - if any of the fold constructors return false, we can
	// exit immediately


	// build up a bijection between IDs as we go
	boost::bimap<size_t, size_t> id_map;

	auto free_func = [&id_map](size_t id1, size_t id2)
	{
		auto left_iter = id_map.left.find(id1);
		auto right_iter = id_map.right.find(id2);

		if (left_iter == id_map.left.end() &&
			right_iter == id_map.right.end())
		{
			id_map.left.insert(std::make_pair(id1, id2));
			return true;
		}
		else if (left_iter != id_map.left.end())
		{
			return left_iter->second == id2;
		}
		else
		{
			return right_iter->second == id1;
		}
	};

	// now proceed with a similar fold to the generic one but without
	// a result stack or "seen_stack"

	std::vector<std::pair<size_t, size_t>> stack;
	std::vector<std::pair<SyntaxNodeType,
		SyntaxNodeType>> stack_types;

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	// we only go as deep as the shallowest tree
	const size_t capacity = std::min(m_height, other.m_height);
#else
	// we only go as big as the smaller tree (+1 for root)
	const size_t capacity = std::min(m_tree.size(),
		other.m_tree.size()) + 1;
#endif

	stack.reserve(capacity);
	stack_types.reserve(capacity);

	stack.emplace_back(m_tree.root_id(),
		other.m_tree.root_id());
	stack_types.emplace_back(m_tree.root_type(),
		other.m_tree.root_type());

	while (!stack.empty())
	{
		ATP_LOGIC_ASSERT(stack_types.size() == stack.size());

		auto [id1, id2] = stack.back();
		stack.pop_back();
		auto [type1, type2] = stack_types.back();
		stack_types.pop_back();

		if (type1 != type2)
			return false;

		switch (type1)
		{
		case SyntaxNodeType::FREE:
			if (!free_func(id1, id2))
				return false;
			break;

		case SyntaxNodeType::FUNC:
		{
			if (m_tree.func_symb_id(id1) !=
				other.m_tree.func_symb_id(id2))
				return false;

			// get some info about us:

			const auto arity = m_tree.func_arity(id1);

			// if two functions agree on their symbol then they
			// should agree on their arity
			ATP_LOGIC_ASSERT(arity == other.m_tree.func_arity(id2));

			const std::array<size_t, MAX_ARITY>&
				children_a =
				m_tree.func_children(id1);
			const std::array<SyntaxNodeType, MAX_ARITY>&
				child_types_a =
				m_tree.func_child_types(id1);

			const std::array<size_t, MAX_ARITY>&
				children_b = other.m_tree.func_children(
					id2);
			const std::array<SyntaxNodeType, MAX_ARITY>&
				child_types_b =
				other.m_tree.func_child_types(
					id2);

#ifdef ATP_LOGIC_DEFENSIVE
			const size_t size_before
				= stack.size();
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
				std::back_inserter(stack),
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
				std::back_inserter(stack_types),
				[](boost::tuple<SyntaxNodeType,
					SyntaxNodeType> tup)
				{ return std::make_pair(tup.get<0>(),
					tup.get<1>()); });

#ifdef ATP_LOGIC_DEFENSIVE
			ATP_LOGIC_ASSERT(stack.size() ==
				size_before + arity);
#endif
		}
			break;

		case SyntaxNodeType::CONSTANT:
			if (id1 != id2)
				return false;
			break;

		default:
			ATP_LOGIC_ASSERT(false && "Invalid node type.");
			throw std::exception();
		}
	}

	return true;
}


bool Expression::identical(const Expression& other) const
{
	auto free_func = (phxarg::arg1 == phxarg::arg2);

	auto const_func = (phxarg::arg1 == phxarg::arg2);

	auto f_func = [](size_t id1, size_t id2,
		std::vector<bool>::iterator begin,
		std::vector<bool>::iterator end)
	{
		return id1 == id2 && std::all_of(begin, end,
			phxarg::arg1);
	};

	return fold_pair<bool>(free_func, const_func, f_func, false,
		other);
}


bool Expression::try_match(const Expression& expr,
	FreeVarMap<Expression>* p_out_subs) const
{
	// if p_out_subs is null then, to avoid repeating code, we create
	// a map to put there instead
	// (use a unique_ptr for the placeholder to ensure it gets
	// deleted if it exists).
	std::unique_ptr<FreeVarMap<Expression>> _p_placeholder;
	if (p_out_subs == nullptr)
	{
		_p_placeholder = std::make_unique<
			FreeVarMap<Expression>>();
		p_out_subs = _p_placeholder.get();
	}
	else p_out_subs->clear();  // clear this to begin with

	auto free_constructor = [this, p_out_subs](size_t id_a,
		size_t id_b)
		-> bool
	{
		auto iter = p_out_subs->find(id_a);

		// this variable hasn't been mapped yet, we are free to
		// assign it any value we like
		if (iter == p_out_subs->end())
		{
			p_out_subs->insert(id_a, Expression(m_ctx,
				id_b, SyntaxNodeType::FREE));
			return true;
		}
		// this variable has already been mapped; check that the
		// value it was mapped to agrees with this
		else
		{
			// optimised from `return iter->second.equivalent(
			// Expression(m_ctx, id_b, SyntaxNodeType::FREE));`
			return (iter.second().m_tree.root_type() ==
				SyntaxNodeType::FREE);
		}
	};

	// both constants must agree in order for a mapping to exist,
	// even if they don't influence what that mapping is
	auto const_constructor = (phxarg::arg1 == phxarg::arg2);

	auto func_constructor = [](size_t id_a, size_t id_b,
		std::vector<bool>::iterator child_begin,
		std::vector<bool>::iterator child_end)
	{
		return (id_a == id_b) && std::all_of(child_begin,
			child_end, phxarg::arg1);
	};

	auto default_constructor = [this, p_out_subs](Expression
		expr_left, Expression expr_right)
	{
		if (expr_left.m_tree.root_type() != SyntaxNodeType::FREE)
			return false;

		const size_t free_id = expr_left.m_tree.root_id();

		auto iter = p_out_subs->find(free_id);

		// this variable hasn't been mapped yet, we are free to
		// assign it any value we like
		if (iter == p_out_subs->end())
		{
			p_out_subs->insert(free_id, std::move(expr_right));
			return true;
		}
		// this variable has already been mapped; check that the
		// value it was mapped to agrees with this
		else return iter.second().equivalent(expr_right);
	};

	const bool success = fold_pair<bool>(free_constructor,
		const_constructor, func_constructor, default_constructor,
		expr);

	// ensure that we don't leave any garbage data in the user output
	// if we fail to build a mapping
	if (!success)
		p_out_subs->clear();

	return success;
}


std::pair<size_t, SyntaxNodeType>
Expression::add_tree_data(const SyntaxNodePtr& tree)
{
	ATP_LOGIC_PRECOND(tree->get_type() !=
		SyntaxNodeType::EQ);

	typedef std::pair<size_t, SyntaxNodeType> NodePair;

	auto eq_func = [](NodePair l, NodePair r) -> NodePair
	{
		ATP_LOGIC_PRECOND(false && "no eq allowed!");

		// we have to return something to avoid compiler
		// errors
		return std::make_pair(0, SyntaxNodeType::EQ);
	};

	auto free_func = [this](size_t free_id) -> NodePair
	{
		return std::make_pair(free_id, SyntaxNodeType::FREE);
	};

	auto const_func = [](size_t symb_id) -> NodePair
	{
		return std::make_pair(symb_id, SyntaxNodeType::CONSTANT);
	};

	auto f_func = [this](size_t symb_id,
		std::vector<NodePair>::iterator begin,
		std::vector<NodePair>::iterator end) -> NodePair
	{
		const size_t arity = std::distance(begin, end);

		auto map_first = boost::bind(&NodePair::first, _1);
		auto map_second = boost::bind(&NodePair::second, _1);

		auto child_begin = boost::make_transform_iterator(begin,
			map_first);
		auto child_end = boost::make_transform_iterator(end,
			map_first);
		auto child_type_begin = boost::make_transform_iterator(begin,
			map_second);
		auto child_type_end = boost::make_transform_iterator(end,
			map_second);

		const size_t func_idx = m_tree.add_func(symb_id, arity,
			child_begin, child_end, child_type_begin, child_type_end);

		return std::make_pair(func_idx, SyntaxNodeType::FUNC);
	};

	return fold_syntax_tree<NodePair>(eq_func, free_func, const_func,
		f_func, tree);
}


#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
size_t Expression::compute_height() const
{
	// this is another instance of a fold
	// however, again, this function is called a lot so needs to
	// be as fast as possible
	// it also can't rely on the height for estimating how much
	// capacity the stack should be given, to reduce allocations.

	// this can be done more efficiently than with a fold because
	// we don't need to a pass back down again - we don't need
	// a "result stack".

	std::vector<size_t> id_stack, height_stack;
	std::vector<SyntaxNodeType> type_stack;
	size_t max_height = 0;

	// how much to reserve initially in these stacks? m_tree.size()
	// is a good, but loose, upper bound. +1 for the root.

	const size_t capacity = m_tree.size() + 1;

	id_stack.reserve(capacity);
	height_stack.reserve(capacity);
	type_stack.reserve(capacity);

	// add root
	id_stack.push_back(m_tree.root_id());
	type_stack.push_back(m_tree.root_type());
	height_stack.push_back(1);  // root has height 1

	while (!id_stack.empty())
	{
		ATP_LOGIC_ASSERT(id_stack.size() == type_stack.size());
		ATP_LOGIC_ASSERT(id_stack.size() == height_stack.size());

		const size_t id = id_stack.back();
		id_stack.pop_back();
		const auto type = type_stack.back();
		type_stack.pop_back();
		const size_t h = height_stack.back();
		height_stack.pop_back();

		// check if the height we just reached is the tallest seen
		// so far:
		if (h > max_height)
			max_height = h;

		if (type == SyntaxNodeType::FUNC)
		{
			// get some info about us:

			const size_t arity = m_tree.func_arity(id);
			const std::array<size_t, MAX_ARITY>&
				children = m_tree.func_children(id);
			const std::array<SyntaxNodeType, MAX_ARITY>&
				child_types = m_tree.func_child_types(id);

			// push children onto stack

			id_stack.insert(id_stack.end(),
				children.begin(), children.begin() + arity);
			type_stack.insert(type_stack.end(),
				child_types.begin(), child_types.begin() + arity);
			height_stack.resize(height_stack.size() + arity,
				h + 1);
		}
	}

	return max_height;
}
#endif


void Expression::build_free_var_ids() const
{
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	ATP_LOGIC_PRECOND(m_height > 0);  // ensure this is set too
#endif
	ATP_LOGIC_PRECOND(!m_free_var_ids.has_value());

	m_free_var_ids = FreeVarIdSet();

	std::vector<std::pair<size_t, SyntaxNodeType>> stack;

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
	const size_t capacity = m_height;
#else
	const size_t capacity = m_tree.size();
#endif
	stack.reserve(capacity);

	stack.emplace_back(m_tree.root_id(), m_tree.root_type());

	while (!stack.empty())
	{
		auto [id, type] = stack.back();
		stack.pop_back();

		switch (type)
		{
		case SyntaxNodeType::FREE:
			m_free_var_ids->insert(id);
			break;
		case SyntaxNodeType::CONSTANT:
			break;
		case SyntaxNodeType::FUNC:
			// just add the function's arguments to the stack
			for (size_t i = 0; i < m_tree.func_arity(id); ++i)
			{
				stack.emplace_back(m_tree.func_children(id).at(i),
					m_tree.func_child_types(id).at(i));
			}
			break;
		default:
			ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
			throw std::exception();
		}
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


