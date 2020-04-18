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
}


Statement::Statement(const ModelContext& ctx, Expression lhs,
	Expression rhs) :
	m_ctx(ctx),
	m_sides(Expression::construct(std::move(lhs)),
		Expression::construct(std::move(rhs)))
{
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


const FreeVarIdSet& Statement::free_var_ids() const
{
	if (!m_free_var_ids.has_value())
		build_free_var_ids();

	return m_free_var_ids.get();
}


Statement Statement::map_free_vars(const std::map<size_t,
	Expression> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(free_var_ids().begin(),
		free_var_ids().end(),
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

	new_stmt.m_free_var_ids = boost::none;

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


	// this function is absolutely an instance of a fold, but we can
	// optimise it beyond the generic fold implementation (and this
	// is important because this function is called a lot)
	// the optimisation is just because we don't need the "result
	// stack" - if any of the fold constructors return false, we can
	// exit immediately


	// build up a bijection between IDs as we go
	// (note that we are attempting two directions of equivalence
	// checking simultaneously - matching l2l and r2r, and also
	// trying to match the transpose, l2r and r2l)
	boost::bimap<size_t, size_t> id_maps[2];

	auto free_func = [&id_maps](size_t j, size_t id1,
		size_t id2)
	{
		ATP_LOGIC_ASSERT(j < 2);
		auto& map = id_maps[j];

		auto left_iter = map.left.find(id1);
		auto right_iter = map.right.find(id2);

		if (left_iter == map.left.end() &&
			right_iter == map.right.end())
		{
			map.left.insert(std::make_pair(id1, id2));
			return true;
		}
		else if (left_iter != map.left.end())
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

	struct StackFrame
	{
		StackFrame(size_t j, size_t id1, size_t id2,
			SyntaxNodeType type1, SyntaxNodeType type2,
			const ExprTreeFlyweight& tree1,
			const ExprTreeFlyweight& tree2) :
			j(j), id1(id1), id2(id2), type1(type1), type2(type2),
			tree1(tree1), tree2(tree2)
		{ }

		size_t j;  // 0 or 1 indicating transpose or not
		size_t id1, id2;
		SyntaxNodeType type1, type2;
		const ExprTreeFlyweight& tree1;
		const ExprTreeFlyweight& tree2;
	};

	std::vector<StackFrame> stack;

	// to track how we are doing on either side

	bool failed_equiv[2] = { false, false };  // transpose

	// add starting values

	const Expression* my_exprs[2] =
	{ m_sides.first.get(), m_sides.second.get() };
	const Expression* other_exprs[2] =
	{ other.m_sides.first.get(), other.m_sides.second.get() };

	stack.reserve(4);

	size_t max_st_size = 0;  // get upper bound on stack size

	for (size_t my_i = 0; my_i < 2; ++my_i)
	{
		for (size_t other_i = 0; other_i < 2; ++other_i)
		{
			// for example, when both i's are 0, this represents
			// trying to match the LHS of this to LHS of other.
			const size_t is_transposed = (my_i != other_i) ?
				1 : 0;

			// clever trick: if the height of these expressions is
			// different, they are not equivalent!
			
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
			const size_t height = my_exprs[my_i]->height();
#endif
			if (
#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
				height == other_exprs[other_i]->height() &&
#endif
				!failed_equiv[is_transposed])
			{
				stack.emplace_back(is_transposed,
					my_exprs[my_i]->tree().root_id(),
					other_exprs[other_i]->tree().root_id(),
					my_exprs[my_i]->tree().root_type(),
					other_exprs[other_i]->tree().root_type(),
					my_exprs[my_i]->tree(),
					other_exprs[other_i]->tree());

#ifdef ATP_LOGIC_EXPR_USE_HEIGHT
				max_st_size += height;
#else
				max_st_size += std::min(
					my_exprs[my_i]->tree().size(),
					other_exprs[other_i]->tree().size());
#endif
			}
			else
			{
				// the two expressions have different heights, so
				// cannot be equivalent, so this check has failed.
				failed_equiv[is_transposed] = true;
			}
		}
	}

	stack.reserve(max_st_size);

	auto notify_fail = [&failed_equiv](size_t j)
	{
		ATP_LOGIC_ASSERT(j < 2);

		// mark this side as failed
		failed_equiv[j] = true;

		// Okay to exit iff both attempts have failed
		return (failed_equiv[0] && failed_equiv[1]);
	};

	while (!stack.empty())
	{
		const auto st_frame = stack.back();
		stack.pop_back();

		if (failed_equiv[st_frame.j])
			continue;  // don't bother checking

		if (st_frame.type1 != st_frame.type2)
		{
			if (notify_fail(st_frame.j))
				return false;
			else
			{
				// skip the rest because this node still failed,
				// even though we haven't halted the whole
				// operation yet
				continue;
			}
		}
		else switch (st_frame.type1)
		{
		case SyntaxNodeType::FREE:
			if (!free_func(st_frame.j, st_frame.id1,
				st_frame.id2))
			{
				if (notify_fail(st_frame.j))
					return false;
			}
			break;

		case SyntaxNodeType::FUNC:
		{
			if (st_frame.tree1.func_symb_id(st_frame.id1) !=
				st_frame.tree2.func_symb_id(st_frame.id2))
			{
				if (notify_fail(st_frame.j))
					return false;
				else
				{
					// skip the rest because this node still failed,
					// even though we haven't halted the whole
					// operation yet
					break;
				}
			}

			// get some info about us:

			const auto arity = st_frame.tree1.func_arity(
				st_frame.id1);

			// if two functions agree on their symbol then they
			// should agree on their arity
			ATP_LOGIC_ASSERT(arity == st_frame.tree2.func_arity(
				st_frame.id2));

			const std::array<size_t, MAX_ARITY>&
				children_a =
				st_frame.tree1.func_children(st_frame.id1);
			const std::array<SyntaxNodeType, MAX_ARITY>&
				child_types_a =
				st_frame.tree1.func_child_types(st_frame.id1);

			const std::array<size_t, MAX_ARITY>&
				children_b = st_frame.tree2.func_children(
					st_frame.id2);
			const std::array<SyntaxNodeType, MAX_ARITY>&
				child_types_b =
				st_frame.tree2.func_child_types(
					st_frame.id2);

			// now add children
			// (warning: don't forget that those arrays
			// aren't necessarily full!)

			for (size_t k = 0; k < arity; ++k)
			{
				stack.emplace_back(
					st_frame.j, children_a[k],
					children_b[k], child_types_a[k],
					child_types_b[k], st_frame.tree1,
					st_frame.tree2);
			}
		}
		break;

		case SyntaxNodeType::CONSTANT:
			if (st_frame.id1 != st_frame.id2)
			{
				if (notify_fail(st_frame.j))
					return false;
			}
			break;

		default:
			ATP_LOGIC_ASSERT(false && "Invalid node type.");
			throw std::exception();
		}
	}

	return !failed_equiv[0] || !failed_equiv[1];
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


void Statement::build_free_var_ids() const
{
	ATP_LOGIC_PRECOND(m_sides.first != nullptr);
	ATP_LOGIC_PRECOND(m_sides.second != nullptr);
	ATP_LOGIC_PRECOND(!m_free_var_ids.has_value());

	m_free_var_ids = m_sides.first->free_var_ids();

	const auto& second_ids = m_sides.second->free_var_ids();

	for (auto id : second_ids)
	{
		m_free_var_ids->insert(id);
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


