/**
\file

\author Samuel Barrett

*/


#include <boost/phoenix.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include "../ATPStatsMinAssignment.h"
#include "EquationalEditDistance.h"


namespace phxargs = boost::phoenix::arg_names;
namespace ublas = boost::numeric::ublas;
using atp::logic::equational::SyntaxNodeType;


namespace atp
{
namespace stats
{


EquationalEditDistanceTracker::EquationalEditDistanceTracker(
	float match_benefit, float unmatch_cost) :
	m_match_benefit(match_benefit), m_unmatch_cost(unmatch_cost)
{
	ATP_STATS_PRECOND(m_match_benefit >= 0.0f);
	ATP_STATS_PRECOND(m_unmatch_cost >= 0.0f);
}


float EquationalEditDistanceTracker::edit_distance(
	const logic::IStatement& stmt1, const logic::IStatement& stmt2)
{
	auto p_stmt1 = dynamic_cast<const logic::equational::Statement*>(&stmt1);
	auto p_stmt2 = dynamic_cast<const logic::equational::Statement*>(&stmt2);

	ATP_STATS_PRECOND(p_stmt1 != nullptr);
	ATP_STATS_PRECOND(p_stmt2 != nullptr);

	// try both the cost and its transpose
	const float cost =
		edit_distance(p_stmt1->lhs(), p_stmt2->lhs(), 0)
		+ edit_distance(p_stmt1->rhs(), p_stmt2->rhs(), 0);

	const float transpose_cost =
		edit_distance(p_stmt1->lhs(), p_stmt2->rhs(), 0)
		+ edit_distance(p_stmt1->rhs(), p_stmt2->lhs(), 0);

	// return the best of the two:
	return std::min(cost, transpose_cost);
}


std::vector<std::vector<float>> EquationalEditDistanceTracker::edit_distance(
	const logic::IStatementArray& stmtarr1,
	const logic::IStatementArray& stmtarr2)
{
	std::vector<std::vector<float>> result;

	auto p_stmtarr1 = dynamic_cast<const logic::equational::StatementArray*>(
		&stmtarr1);
	auto p_stmtarr2 = dynamic_cast<const logic::equational::StatementArray*>(
		&stmtarr2);

	ATP_STATS_PRECOND(p_stmtarr1 != nullptr);
	ATP_STATS_PRECOND(p_stmtarr2 != nullptr);

	result.resize(stmtarr1.size(),
		std::vector<float>(stmtarr2.size()));

	for (size_t i = 0; i < stmtarr1.size(); ++i)
	{
		const auto& stmt1 = p_stmtarr1->my_at(i);
		const Expression stmt1expr[2] = {
			stmt1.lhs(), stmt1.rhs() };

		for (size_t j = 0; j < stmtarr2.size(); ++j)
		{
			const auto& stmt2 = p_stmtarr2->my_at(j);

			const Expression stmt2expr[2] = {
				stmt2.lhs(), stmt2.rhs() };

			// try both the cost and its transpose
			const float cost =
				edit_distance(stmt1expr[0], stmt2expr[0], 0)
				+ edit_distance(stmt1expr[1], stmt2expr[1], 0);

			const float transpose_cost =
				edit_distance(stmt1expr[0], stmt2expr[1], 0)
				+ edit_distance(stmt1expr[1], stmt2expr[0], 0);

			// pick the best of the two:
			result[i][j] = std::min(cost, transpose_cost);
		}
	}

	return result;
}


std::vector<std::vector<std::vector<float>>>
EquationalEditDistanceTracker::sub_edit_distance(
	const logic::IStatementArray& stmtarr1,
	const logic::IStatementArray& stmtarr2)
{
	std::vector<std::vector<std::vector<float>>> result;

	auto p_stmtarr1 = dynamic_cast<const logic::equational::StatementArray*>(
		&stmtarr1);
	auto p_stmtarr2 = dynamic_cast<const logic::equational::StatementArray*>(
		&stmtarr2);

	ATP_STATS_PRECOND(p_stmtarr1 != nullptr);
	ATP_STATS_PRECOND(p_stmtarr2 != nullptr);

	result.resize(stmtarr1.size(),
		std::vector<std::vector<float>>(stmtarr2.size()));

	for (size_t i = 0; i < stmtarr1.size(); ++i)
	{
		const auto& stmt1 = p_stmtarr1->my_at(i);
		std::vector<Expression> sub_exprs;
		for (auto iter = stmt1.begin(); iter != stmt1.end(); ++iter)
		{
			// skip free variables
			if (iter->root_type() ==
				logic::equational::SyntaxNodeType::FREE)
				continue;

			sub_exprs.emplace_back(std::move(*iter));
		}

		for (size_t j = 0; j < stmtarr2.size(); ++j)
		{
			const auto& stmt2 = p_stmtarr2->my_at(j);

			const Expression stmt2expr[2] = {
				stmt2.lhs(), stmt2.rhs() };

			result[i][j].resize(sub_exprs.size());

			// heuristic: if we loop over backwards, we are less
			// likely to get cache misses
			for (size_t _k = 0; _k < sub_exprs.size(); ++_k)
			{
				const size_t k = sub_exprs.size() - _k - 1;

				// pick the best of the two sides of stmt2:
				result[i][j][k] = std::min(
					edit_distance(sub_exprs[k], stmt2expr[0], 0),
					edit_distance(sub_exprs[k], stmt2expr[1], 0));
			}
		}
	}

	return result;
}


float EquationalEditDistanceTracker::edit_distance(
	const Expression& expr1, const Expression& expr2, size_t depth)
{
	const size_t pair_hash = hash_exprs(expr1, expr2);

	// preprocessing step - try a lookup right at the start
	{
		auto pre_iter = m_dists.find(pair_hash);

		if (pre_iter != m_dists.end())
		{
			return pre_iter->second;  // don't re-do work
		}
	}

	// else need to construct it from its children:
	if (expr1.root_type() == SyntaxNodeType::FREE
		|| expr2.root_type() == SyntaxNodeType::FREE)
	{
		// frees can match with anything - it costs 0 to match with
		// another free, and 1 to match with anything else
		const float cost = (expr1.root_type() == expr2.root_type()) ?
			0.0f : 1.0f;
		m_dists[pair_hash] = cost;
		return cost;
	}
	else if (expr1.root_type() == SyntaxNodeType::CONSTANT)
	{
		// note: expr1 must appear on the RHS of any `m_sub_costs` calls
		// because we know it is a constant thus has arity 0, but
		// expr2 could have any arity.

		// add the cost:
		const float cost = (expr1.root_id() == expr2.root_id()) ?
			(-m_match_benefit) : m_unmatch_cost;
		m_dists[pair_hash] = cost;
		return cost;
	}
	else if (expr2.root_type() == SyntaxNodeType::CONSTANT)
	{
		// note: expr2 must appear on the RHS of any `m_sub_costs` calls
		// because we know it is a constant thus has arity 0, but
		// expr1 could have any arity.

		// add the cost:
		const float cost = (expr1.root_id() == expr2.root_id()) ?
			(-m_match_benefit) : m_unmatch_cost;
		m_dists[pair_hash] = cost;
		return cost;
	}
	else
	{
		// else handle the case with two functions
		ATP_STATS_ASSERT(expr1.root_type() == SyntaxNodeType::FUNC
			&& expr2.root_type() == SyntaxNodeType::FUNC);

		// get arity of both sides:
		const size_t expr1_arity = expr1.tree().func_arity(
			expr1.tree().root_id());
		const size_t expr2_arity = expr2.tree().func_arity(
			expr2.tree().root_id());

		ATP_STATS_ASSERT(expr1_arity > 0 && expr2_arity > 0);

		// construct sub expressions for inner loop out here:
		std::vector<Expression> subexprs2;
		subexprs2.reserve(expr2_arity);
		for (size_t i = 0; i < expr2_arity; ++i)
			subexprs2.emplace_back(expr2.sub_expression(
				expr2.tree().func_children(
					expr2.tree().root_id()).at(i),
				expr2.tree().func_child_types(
					expr2.tree().root_id()).at(i)));

		// construct distance matrix
		ublas::matrix<float> dist_mat(expr1_arity, expr2_arity);
		for (size_t i = 0; i < expr1_arity; ++i)
		{
			const auto subexpr1 = expr1.sub_expression(
				expr1.tree().func_children(
					expr1.tree().root_id()).at(i),
				expr1.tree().func_child_types(
					expr1.tree().root_id()).at(i));

			for (size_t j = 0; j < expr2_arity; ++j)
			{
				// RECURSE
				dist_mat(i, j) = edit_distance(
					subexpr1, subexprs2[j], depth + 1);
			}
		}
		// compute substitution cost (but need to get it the right
		// way around)
		float sub_cost = (expr1.root_id() == expr2.root_id()) ?
			(-m_match_benefit) : m_unmatch_cost;
		if (expr1_arity < expr2_arity)
		{
			// need to transpose this:
			dist_mat = ublas::trans(dist_mat);
		}
		
		// now our value is just the best assignment of the children
		// plus the substitution cost:
		const float cost = sub_cost + minimum_assignment(dist_mat);
		m_dists[pair_hash] = cost;
		return cost;
	}
}



size_t EquationalEditDistanceTracker::hash_expr(const Expression& expr) const
{
	// prime numbers in this function are arbitrary

	// must NOT depend on actual ID (we want it to be invariant to
	// free variable renaming)
	auto free_func = boost::phoenix::val(2097593);
	auto const_func = phxargs::arg1 * 23209;
	auto f_func = [](size_t id,
		const auto& child_begin, const auto& child_end)
	{
		size_t hash = id * 524287;
		for (auto iter = child_begin; iter != child_end; ++iter)
		{
			hash = 486187739 * hash + 263167 * (*iter);
		}
		return hash;
	};

	return expr.fold<size_t>(free_func, const_func, f_func);
}


size_t EquationalEditDistanceTracker::hash_exprs(const Expression& expr1,
	const Expression& expr2) const
{
	const size_t a = hash_expr(expr1),
		b = hash_expr(expr2);

	// it is important that this function is symmetric in a and b
	return (a ^ b) + (b ^ a);
}


}  // namespace stats
}  // namespace atp


