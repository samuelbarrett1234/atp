#pragma once


/*

IStatement.h

This file contains an interface to a class representing a
single statement (for example, an odd number plus an odd
number is even.) These can be thought of as abstract syntax
trees (ASTs). A canonical statement is either the statement
"true", which is trivially true, or "false", which is trivially
false. No other statement is canonical.

*/


#include <string>
#include "../ATPLogicAPI.h"


namespace atp
{
namespace logic
{


/// <summary>
/// Every statement is either canonical or not. It is either true or
/// false if and only if it is canonical. Non-canonical statements
/// require evaluation to decide if they are true or false - and this
/// is exactly the job of a theorem prover! (Reducing non-canonical
/// statements to canonically true or false.)
/// </summary>
enum class StmtForm
{
	NOT_CANONICAL,
	CANONICAL_TRUE,
	CANONICAL_FALSE
};


/// <summary>
/// Statement objects are immutable.
/// They might be useful for exploring the ASTs, or
/// viewing them as a string, or checking if they are
/// canonical.
/// </summary>
class ATP_LOGIC_API IStatement
{
public:
	virtual ~IStatement() = default;


	virtual StmtForm form() const = 0;
	virtual std::string to_str() const = 0;

	// TODO: allow explicit AST traversal (for example, to use as input
	// for statistical models.)
};


}  // namespace logic
}  // namespace atp


