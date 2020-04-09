#pragma once


/**

\file

\author Samuel Barrett

\brief Several useful functions on Statements, including logical
    inference and logical equivalence.

*/


#include <set>
#include <map>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include "Statement.h"
#include "StatementArray.h"


/**

\namespace atp::logic::equational::semantics

\brief Contains all the functions relating to the semantics (logical
    inference etc) of equational logic.

*/


namespace atp
{
namespace logic
{
namespace equational
{
namespace semantics
{


/**
\brief Try applying each rule to the statement to get a list of
    successor statements. Note that each rule may generate many
	possible successors, or none.

\remark The successors returned here are all "iff" in the sense that
    `stmt` will also appear in \a their successors.
*/
ATP_LOGIC_API StatementArray get_successors(const Statement& stmt,
	const std::vector<Statement>& rules);


/**
\brief Returns true iff the syntax trees are equal up to swapping
    of free variable names and reflection about the equals sign
*/
ATP_LOGIC_API bool equivalent(const Statement& a,
	const Statement& b);


/**
\brief Returns true iff the two syntax trees are identical (i.e. same
    free variable IDs, same orientation about equals sign, etc.)
*/
ATP_LOGIC_API bool identical(const Statement& a,
	const Statement& b);


/**
\brief Reflect the statement about the equals sign (so `f(x)=g(x)`
    becomes `g(x)=f(x)`)
*/
ATP_LOGIC_API Statement transpose(const Statement& stmt);


/**
\brief Returns true iff the statement is symmetric about the equals
    sign (thus trivially true by reflexivity of '=').
*/
ATP_LOGIC_API bool true_by_reflexivity(const Statement& stmt);


/**
\brief Returns true iff there exists a substitution in the premmise
    which would produce the conclusion

\warning This is not used in finding proofs, because in this
    implementation of equational logic, proofs are "iff", i.e.
    readable in both directions.
*/
ATP_LOGIC_API bool implies(const Statement& premise,
	const Statement& concl);


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


