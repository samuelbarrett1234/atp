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


size_t EquationalEditDistanceTracker::ExprHashFunc::operator()(
	const Expression& expr) const
{
	// prime numbers in this function are arbitrary

	// must NOT depend on actual ID
	auto free_func = boost::phoenix::val(1013);
	auto const_func = phxargs::arg1 * 1523;
	auto f_func = [](size_t id,
		auto child_begin, auto child_end)
	{
		size_t hash = id * 2029;
		for (auto iter = child_begin; iter != child_end; ++iter)
		{
			hash = 3037 * hash + 4013 * (*iter);
		}
		return hash;
	};
	return expr.fold<size_t>(free_func, const_func, f_func);
}


size_t EquationalEditDistanceTracker::ExprPairHashFunc::operator()(
	const std::pair<Expression, Expression>& p) const
{
	const size_t a = ehf(p.first),
		b = ehf(p.second);

	// it is important that this function is symmetric in a and b
	return std::max(a ^ b, b ^ a);
}


bool EquationalEditDistanceTracker::ExprPairEqFunc::operator()(
	const std::pair<Expression, Expression>& a,
	const std::pair<Expression, Expression>& b) const
{
	// edit distance is invariant to free variable renaming and
	// flipping about the equals sign - so we should reflect those
	// symmetries in the unordered_map storage for maximum efficiency
	return (a.first.equivalent(b.first) &&
		a.second.equivalent(b.second))
		||
		(a.first.equivalent(b.second) &&
			a.second.equivalent(b.first));
}


EquationalEditDistanceTracker::EquationalEditDistanceTracker(
	EditDistSubCosts sub_costs) : m_sub_costs(std::move(sub_costs))
{ }


float EquationalEditDistanceTracker::edit_distance(
	const logic::IStatement& stmt1, const logic::IStatement& stmt2)
{
	auto p_stmt1 = dynamic_cast<const logic::equational::Statement*>(&stmt1);
	auto p_stmt2 = dynamic_cast<const logic::equational::Statement*>(&stmt2);

	ATP_STATS_PRECOND(p_stmt1 != nullptr);
	ATP_STATS_PRECOND(p_stmt2 != nullptr);

	// try both the cost and its transpose
	const float cost =
		edit_distance(p_stmt1->lhs(), p_stmt2->lhs())
		+ edit_distance(p_stmt1->rhs(), p_stmt2->rhs());

	const float transpose_cost =
		edit_distance(p_stmt1->lhs(), p_stmt2->rhs())
		+ edit_distance(p_stmt1->rhs(), p_stmt2->lhs());

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
		for (size_t j = 0; j < stmtarr2.size(); ++j)
		{
			const auto& stmt1 = p_stmtarr1->my_at(i);
			const auto& stmt2 = p_stmtarr2->my_at(j);

			const Expression stmt1expr[2] = {
				stmt1.lhs(), stmt1.rhs() };
			const Expression stmt2expr[2] = {
				stmt2.lhs(), stmt2.rhs() };

			// try both the cost and its transpose
			const float cost =
				edit_distance(stmt1expr[0], stmt2expr[0])
				+ edit_distance(stmt1expr[1], stmt2expr[1]);

			const float transpose_cost =
				edit_distance(stmt1expr[0], stmt2expr[1])
				+ edit_distance(stmt1expr[1], stmt2expr[0]);

			// pick the best of the two:
			result[i][j] = std::min(cost, transpose_cost);;
		}
	}

	return result;
}


float EquationalEditDistanceTracker::edit_distance(
	const Expression& expr1, const Expression& expr2)
{
	return ensure_is_computed(std::make_pair(expr1, expr2));
}


float EquationalEditDistanceTracker::ensure_is_computed(
	const std::pair<Expression, Expression>& expr_pair)
{
	const auto& expr1 = expr_pair.first;
	const auto& expr2 = expr_pair.second;

	// preprocessing step - try a lookup right at the start
	{
		auto pre_iter = m_dists.find(
			expr_pair);

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
		m_dists[expr_pair] = cost;
		return cost;
	}
	else if (expr1.root_type() == SyntaxNodeType::CONSTANT)
	{
		// check the substitution exists
		ATP_STATS_ASSERT(m_sub_costs.find(std::make_pair(expr2.root_id(),
			expr1.root_id())) != m_sub_costs.end());

		// note: expr1 must appear on the RHS of any `m_sub_costs` calls
		// because we know it is a constant thus has arity 0, but
		// expr2 could have any arity.

		// add the cost:
		const float cost =
			m_sub_costs.at(std::make_pair(expr2.root_id(),
			expr1.root_id()));
		m_dists[expr_pair] = cost;
		return cost;
	}
	else if (expr2.root_type() == SyntaxNodeType::CONSTANT)
	{
		// check the substitution exists
		ATP_STATS_ASSERT(m_sub_costs.find(std::make_pair(expr1.root_id(),
			expr2.root_id())) != m_sub_costs.end());

		// note: expr2 must appear on the RHS of any `m_sub_costs` calls
		// because we know it is a constant thus has arity 0, but
		// expr1 could have any arity.

		// add the cost:
		const float cost =
			m_sub_costs.at(std::make_pair(expr1.root_id(),
				expr2.root_id()));
		m_dists[expr_pair] = cost;
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

		// construct distance matrix
		ublas::matrix<float> dist_mat(expr1_arity, expr2_arity);
		for (size_t i = 0; i < expr1_arity; ++i)
		{
			Expression sub_expr1 = expr1.sub_expression(
				expr1.tree().func_children(
					expr1.tree().root_id()).at(i),
				expr1.tree().func_child_types(
					expr1.tree().root_id()).at(i));

			for (size_t j = 0; j < expr2_arity; ++j)
			{
				Expression sub_expr2 = expr2.sub_expression(
					expr2.tree().func_children(
						expr2.tree().root_id()).at(j),
					expr2.tree().func_child_types(
						expr2.tree().root_id()).at(j));

				auto sub_expr_pair = std::make_pair(sub_expr1,
					std::move(sub_expr2));

				// RECURSE
				dist_mat(i, j) = ensure_is_computed(sub_expr_pair);
			}
		}
		// compute substitution cost (but need to get it the right
		// way around)
		float sub_cost = 0.0f;
		if (expr1_arity < expr2_arity)
		{
			// need to transpose this:
			dist_mat = ublas::trans(dist_mat);

			auto iter = m_sub_costs.find(
				std::make_pair(expr2.root_id(),
					expr1.root_id()));

			ATP_STATS_ASSERT(iter != m_sub_costs.end());

			sub_cost = iter->second;
		}
		else
		{
			auto iter = m_sub_costs.find(
				std::make_pair(expr1.root_id(),
					expr2.root_id()));

			ATP_STATS_ASSERT(iter != m_sub_costs.end());

			sub_cost = iter->second;
		}
		
		// now our value is just the best assignment of the children
		// plus the substitution cost:
		const float cost = sub_cost + minimum_assignment(dist_mat);
		m_dists[expr_pair] = cost;
		return cost;
	}
}


}  // namespace stats
}  // namespace atp


