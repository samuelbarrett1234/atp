/**

\file

\author Samuel Barrett

*/


#include "Expression.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
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
	m_ctx(ctx), m_height(1)
{
	ATP_LOGIC_PRECOND(root_type != SyntaxNodeType::FUNC);

	m_tree.set_root(root_id, root_type);

	m_free_var_ids = std::make_shared<std::set<size_t>>();
	if (root_type == SyntaxNodeType::FREE)
		m_free_var_ids->insert(root_id);
}


Expression::Expression(const ModelContext& ctx,
	const SyntaxNodePtr& p_root) :
	m_ctx(ctx)
{
	ATP_LOGIC_PRECOND(p_root->get_type() !=
		SyntaxNodeType::EQ);

	auto root_data = add_tree_data(p_root);

	// it is important we set this, or `m_tree` will be left in an
	// invalid state
	m_tree.set_root(root_data.first, root_data.second);

	// compute height of the syntax tree using a fold
	m_height = fold<size_t>(
		phx::val(1), phx::val(1), [](size_t,
			std::vector<size_t>::iterator begin,
			std::vector<size_t>::iterator end)
		{ return *std::max_element(begin, end) + 1; });
}


Expression::Expression(const Expression& other) :
	m_ctx(other.m_ctx),
	m_height(other.m_height),
	m_tree(other.m_tree),
	m_free_var_ids(other.m_free_var_ids)
{ }


Expression::Expression(Expression&& other) noexcept :
	m_ctx(other.m_ctx),
	m_height(other.m_height),
	m_tree(std::move(other.m_tree)),
	m_free_var_ids(std::move(other.m_free_var_ids))
{ }


Expression& Expression::operator=(const Expression& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
		m_height = other.m_height;
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
		m_height = other.m_height;
		m_tree = std::move(other.m_tree);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


Expression Expression::map_free_vars(const std::map<size_t,
	Expression> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(m_free_var_ids->begin(),
		m_free_var_ids->end(),
		[&free_map](size_t id)
		{ return free_map.find(id) != free_map.end(); }));

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

		const size_t size_before = new_expr.m_tree.size();

		std::map<size_t, size_t> new_map;
		for (const auto& pair : free_map)
		{
			new_map[pair.first] = new_expr.m_tree.merge_from(
				pair.second.m_tree);
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
					auto sub_type = expr_iter->second.m_tree.root_type();

					new_expr.m_tree.update_func_child(i, j,
						sub_id, sub_type);
				}
			}
		}

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
	ATP_LOGIC_PRECOND(pos.belongs_to(this));

	// handle root position as a special case:
	if (pos == begin())
	{
		return sub_expr;
	}
	else
	{
		Expression result_expr = *this;

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
		}
		default:
			ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
			throw std::exception();
		}

		// rebuild the free variable IDs, as they might've been
		// broken by the substitution
		result_expr.m_free_var_ids->clear();
		for (auto iter = result_expr.begin();
			iter != result_expr.end(); ++iter)
		{
			if (iter->m_tree.root_type() == SyntaxNodeType::FREE)
			{
				result_expr.m_free_var_ids->insert(
					iter->m_tree.root_id());
			}
		}

		return result_expr;
	}
}


Expression Expression::replace_free_with_free(
	size_t initial_id, size_t after_id) const
{
	ATP_LOGIC_PRECOND(m_free_var_ids->find(initial_id) !=
		m_free_var_ids->end());

	Expression new_exp = *this;

	for (size_t i = 0; i < new_exp.m_tree.size(); ++i)
	{
		const size_t arity = new_exp.m_tree.func_arity(i);
		const auto& children = new_exp.m_tree.func_children(i);
		const auto& child_types = new_exp.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE &&
				children[j] == initial_id)
			{
				new_exp.m_tree.update_func_child(i, j, after_id,
					SyntaxNodeType::FREE);
			}
		}
	}

	return new_exp;
}


Expression Expression::replace_free_with_const(
	size_t initial_id, size_t const_symb_id) const
{
	ATP_LOGIC_PRECOND(m_free_var_ids->find(initial_id) !=
		m_free_var_ids->end());

#ifdef ATP_LOGIC_DEFENSIVE
	// check the constant symbol ID is valid
	const auto symb_ids = m_ctx.all_constant_symbol_ids();
	ATP_LOGIC_PRECOND((std::find(symb_ids.begin(),
		symb_ids.end(), const_symb_id) != symb_ids.end()));
#endif

	Expression new_exp = *this;

	for (size_t i = 0; i < new_exp.m_tree.size(); ++i)
	{
		const size_t arity = new_exp.m_tree.func_arity(i);
		const auto& children = new_exp.m_tree.func_children(i);
		const auto& child_types = new_exp.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE &&
				children[j] == initial_id)
			{
				new_exp.m_tree.update_func_child(i, j, const_symb_id,
					SyntaxNodeType::CONSTANT);
			}
		}
	}

	return new_exp;
}


Expression Expression::increment_free_var_ids(size_t inc) const
{
	// handle this special case more efficiently:
	if (inc == 0)
		return *this;

	Expression new_exp = *this;

	for (size_t i = 0; i < new_exp.m_tree.size(); ++i)
	{
		const size_t arity = new_exp.m_tree.func_arity(i);
		const auto& children = new_exp.m_tree.func_children(i);
		const auto& child_types = new_exp.m_tree.func_child_types(i);
		for (size_t j = 0; j < arity; ++j)
		{
			if (child_types[j] == SyntaxNodeType::FREE)
			{
				new_exp.m_tree.update_func_child(i, j,
					children[j] + inc, SyntaxNodeType::FREE);
			}
		}
	}

	return new_exp;
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
		new_expr.m_tree.set_root(id, type);
		return new_expr;
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type");
		throw std::exception();
	}
}


bool Expression::equivalent(const Expression& other) const
{
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
	std::map<size_t, Expression>* p_out_subs) const
{
	// if p_out_subs is null then, to avoid repeating code, we create
	// a map to put there instead
	// (use a unique_ptr for the placeholder to ensure it gets
	// deleted if it exists).
	std::unique_ptr<std::map<size_t, Expression>> _p_placeholder;
	if (p_out_subs == nullptr)
	{
		_p_placeholder = std::make_unique<
			std::map<size_t, Expression>>();
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
			p_out_subs->at(id_a) = Expression(m_ctx,
				id_b, SyntaxNodeType::FREE);
			return true;
		}
		// this variable has already been mapped; check that the
		// value it was mapped to agrees with this
		else return iter->second.equivalent(Expression(m_ctx,
			id_b, SyntaxNodeType::FREE));
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
			p_out_subs->at(free_id) = expr_right;
			return true;
		}
		// this variable has already been mapped; check that the
		// value it was mapped to agrees with this
		else return iter->second.equivalent(expr_right);
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
		m_free_var_ids->insert(free_id);
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


}  // namespace equational
}  // namespace logic
}  // namespace atp


