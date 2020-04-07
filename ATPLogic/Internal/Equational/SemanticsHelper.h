#pragma once


#include <set>
#include <map>
#include <list>
#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "SyntaxNodes.h"
#include "Statement.h"


namespace atp
{
namespace logic
{
namespace equational
{
namespace semantics
{


// returns true iff the two given syntax trees are identical
ATP_LOGIC_API bool syntax_tree_identical(SyntaxNodePtr a, SyntaxNodePtr b);


/// <summary>
/// Try to apply each rule at the subtree rooted at the given node
/// and return the results
/// </summary>
/// <param name="node">The root of the subtree for which we should
/// try to make the rule application.</param>
/// <param name="rule_exprs">An array of (LHS, RHS) pairs for each
/// rule to apply.</param>
/// <param name="free_var_ids">The set of all free variable IDs
/// present in the statement from which `node` was obtained (all
/// statements returned from this function will only be able to
/// contain free variables from this set, so if you don't provide
/// them all it may be making some successors unreachable.)</param>
/// <param name="rule_free_vars">For each rule, give the set of
/// free variable IDs in it.</param>
ATP_LOGIC_API std::list<SyntaxNodePtr> immediate_applications(
	SyntaxNodePtr node, const std::vector<std::pair<SyntaxNodePtr,
	SyntaxNodePtr>>& rule_exprs,
	const std::set<size_t>& free_var_ids,
	const std::vector<std::set<size_t>>& rule_free_vars);


// try to obtain a free variable mapping which makes the premise
// expression be identical to the conclusion expression (note
// the word "expression" here is meant to mean "without an = sign").
ATP_LOGIC_API boost::optional<std::map<size_t, SyntaxNodePtr>>
	try_build_map(SyntaxNodePtr expr_premise,
		SyntaxNodePtr expr_concl);


// a simple function for applying a substitution to a given tree
// (there may be many ways to substitute a tree, if the substitution
// introduces new free variables!)
/// <summary>
/// Apply a substitution to the subtree rooted at the given node
/// (Of course there may be many ways to do this, so we return a
/// list.)
/// Particular attention must be paid to the case when the
/// substitution would introduce new free variables, as we would then
/// need to patch those up with previously existing free variables.
/// </summary>
/// <param name="node">The root of the subtree for which we should
/// try to make the substitution.</param>
/// <param name="free_var_map">The desired free variable substitution
/// mapping</param>
/// <param name="free_var_ids">The set of all free variable IDs
/// present in the statement from which `node` was obtained (all
/// statements returned from this function will only be able to
/// contain free variables from this set, so if you don't provide
/// them all it may be making some successors unreachable.)</param>
/// <param name="rule_free_ids">The free variable IDs which are
/// present in EITHER SIDE of the formula (of course this will be a
/// superset of the IDs being mapped in `free_var_map`).</param>
ATP_LOGIC_API std::list<SyntaxNodePtr> substitute_tree(
	SyntaxNodePtr node,
	const std::map<size_t, SyntaxNodePtr>& free_var_map,
	const std::set<size_t>& free_var_ids,
	const std::set<size_t>& rule_free_ids);


// get a list containing the LHS and RHS of each statement given as
// input
ATP_LOGIC_API std::vector<std::pair<SyntaxNodePtr, SyntaxNodePtr>>
	get_statement_sides(const std::vector<Statement>& stmts);


// get the free variable IDs present in this node
ATP_LOGIC_API std::set<size_t> get_free_var_ids(SyntaxNodePtr node);


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


