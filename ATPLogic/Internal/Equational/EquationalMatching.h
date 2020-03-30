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
namespace eq_matching
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


}  // namespace eq_matching
}  // namespace logic
}  // namespace atp


