#pragma once


/*

Semantics.h

This file contains code for performing logical inference in
equational logic statements, and also for various other
operations on statements, like checking logical equivalence.

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


// Try applying each given equality rule to the given statement, to
// produce other logically equivalent statements which are
// "adjacent" to `stmt`. Note that the statements returned are "iff".
ATP_LOGIC_API StatementArray get_successors(const Statement& stmt,
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


