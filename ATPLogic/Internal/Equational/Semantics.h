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


// For each input rule, try making a substitution! Using the given
// equality rules, we explore the syntax tree of `stmt` and at each
// syntax node we see if either side of any rule is applicable. If
// so, we determine what assignments should be made for the free
// variables in the rule, and then replace one side with the other.
ATP_LOGIC_API StatementArray get_substitutions(const Statement& stmt,
	const std::vector<Statement>& rules);


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


// returns true iff there exists a substitution in the premise
// which would produce the conclusion (note that this is NOT used
// in finding proofs, because it is a one-way implication.)
ATP_LOGIC_API bool implies(const Statement& premise,
	const Statement& concl);


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


