#pragma once


/*

SemanticsHelper.h

Provides several helper functions for the code in Semantics.h /
Semantics.cpp

*/


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


// This structure holds lots of information for some of the functions
// below (since so much information is required, and caching a lot of
// it is important for performance).
struct ATP_LOGIC_API SubstitutionInfo
{
	SubstitutionInfo(const KnowledgeKernel& kernel,
		const std::vector<Statement>& rules,
		const std::set<size_t>& stmt_free_var_ids);

	// The knowledge kernel from which we obtained the rules etc
	const KnowledgeKernel& kernel;

	// A list of (LHS, RHS) pairs for the rules (they're more useful
	// when split into LHS and RHS)
	std::vector<std::pair<SyntaxNodePtr, SyntaxNodePtr>> rule_exprs;

	// All of the free variable IDs present in the statement for
	// which `node` is a subtree of (it is important that you get
	// all, not just the ones present in the subtree rooted at
	// `node`.
	std::set<size_t> free_var_ids;

	// All free variables present in each rule
	std::vector<std::set<size_t>> rule_free_vars;

	// The symbol ID of every user-defined constant
	std::vector<size_t> const_symbol_ids;
};


// Try to apply each rule at the subtree rooted at the given node
// and return the results
ATP_LOGIC_API std::list<SyntaxNodePtr> immediate_applications(
	SyntaxNodePtr node,
	const SubstitutionInfo& sub_info);


/// <summary>
/// Apply a substitution to the subtree rooted at the given node
/// (Of course there may be many ways to do this, so we return a
/// list.)
/// Particular attention must be paid to the case when the
/// substitution would introduce new free variables, as we would then
/// need to patch those up with previously existing free variables.
/// </summary>
/// <param name="node">The node to substitute into (typically this
/// is the other side of the rule that was matched; see this
/// function's usage in `immediate_applications`.</param>
ATP_LOGIC_API std::list<SyntaxNodePtr> substitute_tree(
	SyntaxNodePtr node,
	const SubstitutionInfo& sub_info,
	const std::map<size_t, SyntaxNodePtr>& free_var_map,
	size_t rule_idx);


// try to obtain a free variable mapping which makes the premise
// expression be identical to the conclusion expression (note
// the word "expression" here is meant to mean "without an = sign").
ATP_LOGIC_API boost::optional<std::map<size_t, SyntaxNodePtr>>
try_build_map(SyntaxNodePtr expr_premise,
	SyntaxNodePtr expr_concl);


// returns true iff the two given syntax trees are identical
ATP_LOGIC_API bool syntax_tree_identical(SyntaxNodePtr a,
	SyntaxNodePtr b);


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


