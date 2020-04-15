/**

\file

\author Samuel Barrett

*/


#include "Statement.h"
#include <boost/bimap.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
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


Statement::Statement(
	const ModelContext& ctx,
	const SyntaxNodePtr& p_root) :
	m_ctx(ctx)
{
	ATP_LOGIC_PRECOND(p_root->get_type() ==
		SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(p_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	m_sides = std::make_pair(Expression::construct(ctx,
		p_eq->left()), Expression::construct(ctx,
			p_eq->right()));

	rebuild_free_var_ids();
}


Statement::Statement(const ModelContext& ctx, Expression lhs,
	Expression rhs) :
	m_ctx(ctx),
	m_sides(Expression::construct(std::move(lhs)),
		Expression::construct(std::move(rhs)))
{
	rebuild_free_var_ids();
}


Statement::Statement(const Statement& other) :
	m_ctx(other.m_ctx)
{
	*this = other;
}


Statement::Statement(Statement&& other) noexcept :
	m_ctx(other.m_ctx)
{
	*this = std::move(other);
}


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
		m_sides = other.m_sides;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Statement& Statement::operator=(Statement&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
		m_sides = std::move(other.m_sides);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


std::string Statement::to_str() const
{
	return m_sides.first->to_str() + " = " +
		m_sides.second->to_str();
}


Statement Statement::map_free_vars(const std::map<size_t,
	Expression> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(m_free_var_ids.begin(),
		m_free_var_ids.end(),
		[&free_map](size_t id)
		{ return free_map.find(id) != free_map.end(); }));

	Statement new_stmt = *this;

	// create a separate copy
	new_stmt.m_sides.first = std::make_shared<Expression>(
		*m_sides.first);
	new_stmt.m_sides.second = std::make_shared<Expression>(
		*m_sides.second);

	*new_stmt.m_sides.first = m_sides.first->map_free_vars(
		free_map);
	*new_stmt.m_sides.second = m_sides.second->map_free_vars(
		free_map);

	new_stmt.rebuild_free_var_ids();

	return new_stmt;
}


Statement Statement::replace(const iterator& pos,
	const Expression& expr) const
{
	ATP_LOGIC_PRECOND(pos != end());
	if (pos.m_left != m_sides.first->end())
	{
		auto expr_lhs = m_sides.first->replace(
			pos.m_left, expr);
		return Statement(m_ctx, std::move(expr_lhs),
			*m_sides.second);
	}
	else
	{
		auto expr_rhs = m_sides.second->replace(
			pos.m_right, expr);
		return Statement(m_ctx, *m_sides.first,
			std::move(expr_rhs));
	}
}


Statement Statement::replace_free_with_free(
	size_t initial_id, size_t after_id) const
{
	return Statement(m_ctx, m_sides.first->replace_free_with_free(
		initial_id, after_id),
		m_sides.second->replace_free_with_free(initial_id,
			after_id));
}


Statement Statement::replace_free_with_const(
	size_t initial_id, size_t const_symb_id) const
{
	return Statement(m_ctx, m_sides.first->replace_free_with_const(
		initial_id, const_symb_id),
		m_sides.second->replace_free_with_const(initial_id,
			const_symb_id));
}


Statement Statement::increment_free_var_ids(
	size_t inc) const
{
	return Statement(m_ctx,
		m_sides.first->increment_free_var_ids(inc),
		m_sides.second->increment_free_var_ids(inc));
}


Statement Statement::transpose() const
{
	Statement tr = *this;

	std::swap(tr.m_sides.first, tr.m_sides.second);

	return tr;
}


bool Statement::equivalent(const Statement& other) const
{
	// unfortunately this is more complicated than calling
	// `Expression::equivalent` on both sides separately, because we
	// need to ensure that the free variable mappings for both sides
	// agree

	// build up a bijection between IDs as we go
	boost::bimap<size_t, size_t> id_map;

	auto eq_func = (phxarg::arg1 && phxarg::arg2);

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

	return fold_pair<bool>(eq_func, free_func, const_func, f_func,
		false, other) || fold_pair<bool>(eq_func, free_func,
			const_func, f_func, false, other.transpose());
	// check transpose case too ^^^
}


bool Statement::implies(const Statement& conclusion) const
{
	// check that the two maps don't conflict on their intersection
	auto maps_compatible = [](
		const std::map<size_t, Expression>& a,
		const std::map<size_t, Expression>& b) -> bool
	{
		// we only need to check consistency between the assignments
		// that are in both a and b, so it suffices to loop over a
		for (const auto& pair : a)
		{
			auto iter = b.find(pair.first);
			if (iter != b.end())
			{
				// we need identical here, rather than equivalent,
				// otherwise: *(x, e) = x => *(*(x, y), e) = *(y, x)
				// for example (there is a test case to cover this).
				if (!pair.second.identical(iter->second))
					return false;
			}
		}
		return true;
	};

	std::map<size_t, Expression> lmap, rmap;

	if (m_sides.first->try_match(*conclusion.m_sides.first,
		&lmap) && m_sides.second->try_match(
		*conclusion.m_sides.second, &rmap))
	{
		if (maps_compatible(lmap, rmap))
			return true;
	}

	// else try again but by matching the transpose

	lmap.clear();
	rmap.clear();

	// the && here short-circuits, and evaluates LHS first, so
	// this is valid
	return m_sides.first->try_match(*conclusion.m_sides.second,
		&lmap) && m_sides.second->try_match(
			*conclusion.m_sides.first, &rmap)
		&& maps_compatible(lmap, rmap);
}


void Statement::rebuild_free_var_ids()
{
	ATP_LOGIC_PRECOND(m_sides.first != nullptr);
	ATP_LOGIC_PRECOND(m_sides.second != nullptr);

	// clear this and rebuild from the union of both children
	m_free_var_ids.clear();

	const auto& first_ids = m_sides.first->free_var_ids();
	const auto& second_ids = m_sides.second->free_var_ids();

	std::set_union(first_ids.begin(), first_ids.end(),
		second_ids.begin(), second_ids.end(),
		std::inserter(m_free_var_ids,
			m_free_var_ids.end()));
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


