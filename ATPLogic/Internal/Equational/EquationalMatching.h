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
ATP_LOGIC_API boost::optional<std::map<size_t, SyntaxNodePtr>>
	try_match(SyntaxNodePtr pattern, SyntaxNodePtr trial);


// returns true iff p_a and p_b are equal as syntax trees,
// which means that they are basically identical except up to
// swapping around the names of the free variables.
// precondition: p_a and p_b are not EQ synax nodes.
ATP_LOGIC_API bool equivalent(SyntaxNodePtr p_a,
	SyntaxNodePtr p_b);


// returns true iff p_a and p_b are identical (i.e. without allowing
// permutations of the free variables.)
// precondition: p_a and p_b are not EQ syntax nodes.
ATP_LOGIC_API bool trivially_equal(SyntaxNodePtr p_a,
	SyntaxNodePtr p_b);


// returns true iff both sides of the equals sign are trivially equal
// (using the above function as a subroutine.)
// precondition: p_eq is an EQ syntax node.
ATP_LOGIC_API bool trivially_true(SyntaxNodePtr p_eq);


}  // namespace eq_matching
}  // namespace logic
}  // namespace atp


