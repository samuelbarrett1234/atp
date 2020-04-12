#pragma once


/**

\file

\author Samuel Barrett

\brief Helper functions for the implementations of the Semantics
    functions

*/


#include <set>
#include <map>
#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "SyntaxNodes.h"
#include "Statement.h"
#include "StatementArray.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ModelContext;  // forward declaration


namespace semantics
{


// This structure holds lots of information for some of the functions
// below (since so much information is required, and caching a lot of
// it is important for performance).
/**
\brief Contains data used in some functions below, bundled together
    for efficiency reasons.

\details The `immediate_applications` and `substitute_tree`
    functions below require a lot of data to run, so to save having
	lots of arguments, and to save recomputing some information
	several times, we bundle it up in this struct which is shared
	between the function calls.
*/
struct ATP_LOGIC_API SubstitutionInfo
{
	SubstitutionInfo(const ModelContext& ctx,
		const StatementArray& rules,
		const std::set<size_t>& stmt_free_var_ids);

	const ModelContext& context;

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

	// for each rule, create a mapping from every free variable in
	// that rule to every free variable in `free_var_ids` union with
	// all possible constants
	// (this work is done once in the constructor of this object
	// instead of being recomputed every time for each call to
	// `substitute_tree`).
	std::vector<std::map<size_t, std::vector<SyntaxNodePtr>>>
		all_var_maps;
};


/**
\brief Try to apply each rule at the subtree rooted at the given node
    without trying tp apply it deeper in the tree (hence "immediate")

\details The term "immediate" comes from the fact that this tries to
    match the rules to the subtree rooted at `node`. So, if `node` is
	a function node with symbol "f", the only rules that can be
	applied are ones which have "f" as their outermost symbol on one
	of their equation sides (in other words, if "f" is a direct child
	of the equality syntax node.)

\param node The subtree root for which we should try a substitution

\pre node->get_type() != SyntaxNodeType::EQ

\see SubstitutionInfo

\returns All of the possible ways of applying each rule to this
    subtree (for each rule, for each side of the equation, for each
	assignment of free variables in the rule, ... etc)
*/
ATP_LOGIC_API std::vector<SyntaxNodePtr> immediate_applications(
	const SyntaxNodePtr& node,
	const SubstitutionInfo& sub_info);



/**
\brief Once we have chosen a rule and found a free variable mapping
    which matches them, this performs the substitution.

\remark Once a free variable mapping has been found which matches a
    side of a rule equation to the subtree given in
	`immediate_applications`, those two expressions would become
	equal, so it would make sense that the only thing we would ever
	want to substitute is the other side of the rule equation! See
	the usage of this function in `immediate_applications`.

\warning This function gets tricky when the substitution doesn't
    provide a value for all free variables. Due to the way we build
	proofs in this equational logic, we cannot introduce new free
	variables in a substitution. Hence if a substitution doesn't
	assign all free variables, we replace the missing ones with other
	things, like free variables which already existed in the (non-
	rule) statement, and constants. This is of course non-exhaustive
	and could make a lot of proofs much harder, especially without an
	array of "helper-theorems". This is kind of important, and might
	be the reason why the prover is failing to prove something that
	should be easy.

\param node The subtree we want to substitute, which should be the
    other side of the rule equation that was matched. See the remark.

\see SubstitutionInfo

\param free_var_map The free variable assignment (this doesn't
    necessarily have to assign all variables; that is the first thing
	to be addressed in the implementation of this function!)

\param rule_idx The index of the rule we are using to substitute in
    the rule array in `sub_info`.

\returns The (potentially many) possible ways of doing this. If all
    free variables are already mapped in `free_var_map` then there
	should only be one way to do the substitution.
*/
ATP_LOGIC_API std::vector<SyntaxNodePtr> substitute_tree(
	const SyntaxNodePtr& node,
	const SubstitutionInfo& sub_info,
	const std::map<size_t, SyntaxNodePtr>& free_var_map,
	size_t rule_idx);


/**
\brief Try to build a substitution of free variables in the premise
    expression, to make it identical to the conclusion expression.

\note "Expression" here is meant to denote the fact that they don't
    have equals signs in them.

\pre expr_premise->get_type() != SyntaxNodeType::EQ and
    expr_concl->get_type() != SyntaxNodeType::EQ

\returns None if no matching substitution was possible, otherwise
    returns the (potentially empty) mapping if one was possible.
*/
ATP_LOGIC_API boost::optional<std::map<size_t, SyntaxNodePtr>>
try_build_map(const SyntaxNodePtr& expr_premise,
	const SyntaxNodePtr& expr_concl);


/**
\returns True iff the two given syntax trees are identical
*/
ATP_LOGIC_API bool syntax_tree_identical(const SyntaxNodePtr& a,
	const SyntaxNodePtr& b);


/**
\brief Get the free variable IDs present in the subtree rooted at
    this node.

\todo Since we know the free variable IDs are dense (quite bounded)
    a `std::set` might not be the best datastructure for this. We
	would be better off using a bitmap datastructure instead.
*/
ATP_LOGIC_API std::set<size_t> get_free_var_ids(const SyntaxNodePtr& node);


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


