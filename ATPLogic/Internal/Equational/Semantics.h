#pragma once


/*

Semantics.h

This file contains the code which determines whether matchings are
possible, between two syntax tree expressions.

*/


#include <set>
#include <map>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include "Statement.h"
#include "StatementArray.h"


namespace atp
{
namespace logic
{
namespace equational
{
namespace semantics
{


// for each input rule, try making a substitution! We do this by
// using the rules as the "patterns" and the `stmt` as the "trial"
// which means that we are looking for ways to substitute the free
// variables in the rules.
// returns an array of at most 4 * rules.size(), because
// statements have two sides each, and 2*2=4!
ATP_LOGIC_API StatementArray get_substitutions(const Statement& stmt,
	const std::vector<Statement>& rules);


// returns all the possible ways a free variable could be
// substituted by a user definition (from the kernel) noting
// that whenever a function is substituted, its arguments
// are always new free variables.
// symb_id_to_arity: a mapping from all symbol IDs, to their
// corresponding arity.
ATP_LOGIC_API StatementArray replace_free_with_def(
	const Statement& stmt,
	const std::map<size_t, size_t>& symb_id_to_arity);


// returns all the ways of replacing a free variable in this
// formula with another free variable in this formula (i.e.
// all the ways you could reduce the number of free variables
// in this formula by 1). Of course, we are considering
// distinct unordered pairs of free variables.
ATP_LOGIC_API StatementArray replace_free_with_free(
	const Statement& stmt);


// returns true iff a and b are equal as syntax trees,
// which means that they are basically identical except UP TO
// swapping around the names of the free variables.
// this DOES include swapping the equals sign around!
ATP_LOGIC_API bool equivalent(const Statement& a,
	const Statement& b);


// returns true iff a and b are identical (i.e. without allowing
// permutations of the free variables.)
ATP_LOGIC_API bool identical(const Statement& a,
	const Statement& b);


// reflect a statement about its equals sign (so "f(x)=g(x)"
// becomes "g(x)=f(x)"
ATP_LOGIC_API Statement transpose(const Statement& stmt);


// returns true iff the statement is trivially true by reflexivity
// of the equals sign (i.e. `stmt` has exactly the same stuff on
// both sides of the equation.)
ATP_LOGIC_API bool true_by_reflexivity(const Statement& stmt);


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


