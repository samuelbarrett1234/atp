#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the IStatement interface for logical statements

\details This file contains an interface to a class representing a
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


/**

\interface IStatement

\brief Represents a single, syntactically correct logical statement.

\details Represents a single, syntactically correct logical
    statement, which is associated with a context (of user-defined
	constants and functions). These objects are designed to be
	immutable; once created, they cannot be modified.
*/
class ATP_LOGIC_API IStatement
{
public:
	virtual ~IStatement() = default;

	virtual std::string to_str() const = 0;

	// TODO: allow explicit AST traversal (for example, to use as input
	// for statistical models.)
};


}  // namespace logic
}  // namespace atp


