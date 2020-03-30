#pragma once


/*

EquationalMatching.h

This file contains the code which determines whether matchings are
possible, between two syntax tree expressions.

*/


#include <map>
#include <boost/optional.hpp>
#include "EquationalSyntaxTrees.h"


namespace atp
{
namespace logic
{
namespace equational
{


typedef std::map<size_t, SyntaxNodePtr> FreeVarSubstitution;


// try to match the trial expression to the pattern expression
// this will be done, if possible, by replacing free variables
// in 'pattern' to match 'trial'.
// this function may be useful for determining if one can apply
// a particular theorem to a statement.
// precondition: pattern and trial are both not SyntaxNodeType::EQ.
// postcondition: returns an assignment of free variable IDs (in
// pattern) to syntax tree expressions which would make 'pattern',
// after the subsitution, equal to 'trial' up to the order of the
// free variables (we don't care about swapping ID's of the free
// variables.)
ATP_LOGIC_API boost::optional<FreeVarSubstitution>
	try_match(SyntaxNodePtr pattern, SyntaxNodePtr trial);


// given a mapping of free variable IDs to subtrees, return a new
// tree which makes the substitutions given (and automatically
// rebuilds the free variable IDs; see `rebuild_free_var_ids`.
ATP_LOGIC_API SyntaxNodePtr get_substitution(SyntaxNodePtr p_node,
	FreeVarSubstitution subs);


// ensure that the free variable IDs in the given syntax tree start
// at 0 and are contiguous (i.e. it cannot be the case that there are
// free variables with IDs 0 and 3 but no free variable with ID 2.)
// warning: this modifies the existing tree object!
// this function is idempotent
ATP_LOGIC_API void rebuild_free_var_ids(SyntaxNodePtr p_node);


// returns true if and only if the tree rooted at p_node has
// contiguous free variable IDs starting at 0 (if any exist).
ATP_LOGIC_API bool needs_free_var_id_rebuild(SyntaxNodePtr p_node);


// returns the number of distinct free variables which are present in
// the subtree rooted at the given node (doesn't have to be a
// contiguous set of variable IDs.)
ATP_LOGIC_API size_t num_free_vars(SyntaxNodePtr p_node);


// returns true iff a and b are equal as syntax trees,
// which means that they are basically identical except UP TO
// swapping around the names of the free variables.
ATP_LOGIC_API bool equivalent(const ISyntaxNode& a,
	const ISyntaxNode& b);


// returns true iff a and b are identical (i.e. without allowing
// permutations of the free variables.)
ATP_LOGIC_API bool identical(const ISyntaxNode& a,
	const ISyntaxNode& b);


// returns true iff both sides of the equals sign are trivially equal
// (using the above function as a subroutine.)
// remark: this function basically tells you whether 'p_eq' follows
// from the reflexivity of =, i.e. the statement "x=x" for free "x".
// precondition: eq is an EQ syntax node.
ATP_LOGIC_API bool trivially_true(const ISyntaxNode& eq);


}  // namespace equational
}  // namespace logic
}  // namespace atp


