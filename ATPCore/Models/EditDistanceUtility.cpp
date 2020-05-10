/**
\file

\author Samuel Barrett

*/


#include <numeric>
#include <vector>
#include <unordered_map>
#include <boost/phoenix.hpp>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Expression.h>
#include "EditDistanceUtility.h"


namespace ublas = boost::numeric::ublas;
namespace phxargs = boost::phoenix::arg_names;
using atp::logic::equational::Expression;
using atp::logic::equational::SyntaxNodeType;


namespace atp
{
namespace core
{


/**
\brief A hash function for equational expressions
*/
struct ExprHashFunc
{
	size_t operator()(const Expression& expr) const
	{
		// prime numbers in this function are arbitrary
		auto free_func = phxargs::arg1 * 1013;
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
};


struct ExprPairHashFunc
{
	ExprHashFunc ehf;

	size_t operator()(const std::pair<Expression,
		Expression>& p) const
	{
		const size_t a = ehf(p.first),
			b = ehf(p.second);

		return a ^ b;
	}
};


struct ExprPairEqFunc
{
	bool operator()(const std::pair<Expression, Expression>& a,
		const std::pair<Expression, Expression>& b) const
	{
		return a.first.identical(b.first) &&
			a.second.identical(b.second);
	}
};


void minimum_assignment_recursive(ublas::matrix<float>& distances,
	std::vector<bool>& in_use, size_t j, float cur_cost,
	float& best_cost_ever)
{
	if (j == distances.size2())
	{
		// no more left
		best_cost_ever = std::min(best_cost_ever,
			cur_cost);

		return;
	}

	for (size_t i = 0; i < in_use.size(); ++i)
	{
		if (!in_use[i])
		{
			in_use[i] = true;

			const float edge_dist = distances(i, j);

			minimum_assignment_recursive(distances,
				in_use, j + 1, cur_cost + edge_dist,
					best_cost_ever);

			in_use[i] = false;
		}
	}
}


float minimum_assignment(ublas::matrix<float>& distances)
{
	const size_t N = distances.size1();
	const size_t M = distances.size2();

	ATP_CORE_PRECOND(N >= M);
	ATP_CORE_PRECOND(M >= 1);

	float best_cost = 0.0f;
	std::vector<bool> in_use;
	in_use.resize(N, false);

	minimum_assignment_recursive(distances,
		in_use, 0, 0.0f, best_cost);

	return best_cost;
}


typedef std::unordered_map<std::pair<Expression, Expression>,
	float, ExprPairHashFunc, ExprPairEqFunc> EditDistMemoisation;


void edit_distance_eq_helper(
	const Expression& expr1,
	const Expression& expr2,
	const EditDistSubCosts& sub_costs,
	EditDistMemoisation& cost_memoisation)
{
	// preprocessing step - try a lookup right at the start
	{
		auto pre_iter = cost_memoisation.find(
			std::make_pair(expr1, expr2));

		if (pre_iter != cost_memoisation.end())
			return;  // don't re-do work
	}

	// else need to construct it from its children:
	if (expr1.root_type() == SyntaxNodeType::FREE
		|| expr2.root_type() == SyntaxNodeType::FREE)
	{
		// frees can match with anything - it costs 0 to match with
		// another free, and 1 to match with anything else
		cost_memoisation[
			std::make_pair(expr1, expr2)] =
			(expr1.root_type() == expr2.root_type()) ?
				0.0f : 1.0f;
	}
	else if (expr1.root_type() == SyntaxNodeType::CONSTANT)
	{
		// check the substitution exists
		ATP_CORE_ASSERT(sub_costs.find(std::make_pair(expr1.root_id(),
			expr2.root_id())) != sub_costs.end());

		// add the cost:
		cost_memoisation[
			std::make_pair(expr1, expr2)] =
			sub_costs.at(std::make_pair(expr1.root_id(),
				expr2.root_id()));
	}
	else if (expr2.root_type() == SyntaxNodeType::CONSTANT)
	{
		// check the substitution exists
		ATP_CORE_ASSERT(sub_costs.find(std::make_pair(expr2.root_id(),
			expr1.root_id())) != sub_costs.end());

		// add the cost:
		cost_memoisation[
			std::make_pair(expr1, expr2)] =
			sub_costs.at(std::make_pair(expr2.root_id(),
				expr1.root_id()));
	}
	else
	{
		// else handle the case with two functions
		ATP_CORE_ASSERT(expr1.root_type() == SyntaxNodeType::FUNC
			&& expr2.root_type() == SyntaxNodeType::FUNC);

		// get arity of both sides:
		const size_t expr1_arity = expr1.tree().func_arity(
			expr1.tree().root_id());
		const size_t expr2_arity = expr2.tree().func_arity(
			expr2.tree().root_id());

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
				Expression sub_expr2 = expr1.sub_expression(
					expr1.tree().func_children(
						expr1.tree().root_id()).at(i),
					expr1.tree().func_child_types(
						expr1.tree().root_id()).at(i));

				edit_distance_eq_helper(sub_expr1,
					sub_expr2, sub_costs, cost_memoisation);

				auto iter = cost_memoisation.find(
					std::make_pair(sub_expr1, sub_expr2));

				ATP_CORE_ASSERT(iter != cost_memoisation.end());

				dist_mat(i, j) = iter->second;
			}
		}

		// compute substitution cost (but need to get it the right
		// way around)
		float sub_cost = 0.0f;
		if (expr1_arity < expr2_arity)
		{
			// need to transpose this:
			dist_mat = ublas::trans(dist_mat);

			auto iter = sub_costs.find(
				std::make_pair(expr2.root_id(),
					expr1.root_id()));

			ATP_CORE_ASSERT(iter != sub_costs.end());

			sub_cost = iter->second;
		}
		else
		{
			auto iter = sub_costs.find(
				std::make_pair(expr1.root_id(),
					expr2.root_id()));

			ATP_CORE_ASSERT(iter != sub_costs.end());

			sub_cost = iter->second;
		}

		// now our value is just the best assignment of the children
		// plus the substitution cost:

		cost_memoisation[
			std::make_pair(expr1, expr2)] =
			sub_cost + minimum_assignment(dist_mat);
	}
}


float edit_distance_eq(
	const Expression& expr1,
	const Expression& expr2,
	const EditDistSubCosts& sub_costs,
	EditDistMemoisation& cost_memoisation)
{
	edit_distance_eq_helper(expr1, expr2, sub_costs,
		cost_memoisation);

	auto iter = cost_memoisation.find(std::make_pair(expr1, expr2));

	ATP_CORE_ASSERT(iter != cost_memoisation.end());

	return iter->second;
}


float edit_distance(
	const logic::IStatement& stmt1,
	const logic::IStatement& stmt2,
	const EditDistSubCosts& sub_costs)
{
	auto p_stmt1 = dynamic_cast<const logic::equational::Statement*>(&stmt1);
	auto p_stmt2 = dynamic_cast<const logic::equational::Statement*>(&stmt2);

	if (p_stmt1 != nullptr && p_stmt2 != nullptr)
	{
		// keep the cost memoisation between calls, to reuse it
		EditDistMemoisation cost_memoisation;

		// try both the cost and its transpose
		const float cost =
			edit_distance_eq(p_stmt1->lhs(), p_stmt2->lhs(),
				sub_costs, cost_memoisation)
			+ edit_distance_eq(p_stmt1->rhs(), p_stmt2->rhs(),
				sub_costs, cost_memoisation);

		const float transpose_cost =
			edit_distance_eq(p_stmt1->lhs(), p_stmt2->rhs(),
				sub_costs, cost_memoisation)
			+ edit_distance_eq(p_stmt1->rhs(), p_stmt2->lhs(),
				sub_costs, cost_memoisation);

		// return the best of the two:
		return std::min(cost, transpose_cost);
	}
	else
	{
		ATP_CORE_ASSERT(false && "bad statement type!");
		ATP_CORE_LOG(fatal) << "Bad logical statement types - "
			"perhaps you forgot to update the rest of the library "
			"with the new logic types?";
		throw std::exception();
	}
}


std::vector<float> pairwise_edit_distance(
	const logic::IStatementArray& stmtarr1,
	const logic::IStatementArray& stmtarr2,
	const EditDistSubCosts& sub_costs)
{
	ATP_CORE_PRECOND(stmtarr1.size() == stmtarr2.size());

	auto p_stmtarr1 = dynamic_cast<
		const logic::equational::StatementArray*>(&stmtarr1);
	auto p_stmtarr2 = dynamic_cast<
		const logic::equational::StatementArray*>(&stmtarr2);

	if (p_stmtarr1 != nullptr && p_stmtarr2 != nullptr)
	{
		// keep the cost memoisation between calls, to reuse it
		EditDistMemoisation cost_memoisation;

		std::vector<float> results;
		results.resize(stmtarr1.size(), 0.0f);

		for (size_t i = 0; i < results.size(); ++i)
		{
			const auto& stmt1 = p_stmtarr1->my_at(i);
			const auto& stmt2 = p_stmtarr2->my_at(i);

			// try both the cost and its transpose
			const float cost =
				edit_distance_eq(stmt1.lhs(), stmt2.lhs(),
					sub_costs, cost_memoisation)
				+ edit_distance_eq(stmt1.rhs(), stmt2.rhs(),
					sub_costs, cost_memoisation);

			const float transpose_cost =
				edit_distance_eq(stmt1.lhs(), stmt2.rhs(),
					sub_costs, cost_memoisation)
				+ edit_distance_eq(stmt1.rhs(), stmt2.lhs(),
					sub_costs, cost_memoisation);

			// return the best of the two:
			results[i] = std::min(cost, transpose_cost);
		}

		return results;
	}
	else
	{
		ATP_CORE_ASSERT(false && "bad statement type!");
		ATP_CORE_LOG(fatal) << "Bad logical statement types - "
			"perhaps you forgot to update the rest of the library "
			"with the new logic types?";
		throw std::exception();
	}
}


}  // namespace core
}  // namespace atp


