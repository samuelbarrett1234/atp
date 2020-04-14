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
#include "SyntaxTreeFold.h"
#include "ModelContext.h"


namespace phx = boost::phoenix;


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


