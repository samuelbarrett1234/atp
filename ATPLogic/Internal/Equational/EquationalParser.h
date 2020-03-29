#pragma once


/*

EquationalParser.h

A parser transforms text into a structured tree of expressions.
In this application, the parser performs no type checking. In
order to use the results of the parser it must first be converted
into a syntax tree (from the parse tree). See the syntax tree code
for more information, or the parse nodes file.

*/


#include <istream>
#include <list>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "EquationalParseNodes.h"


namespace atp
{
namespace logic
{


// Parse an input stream of line-separated statements.
// Return a list of correctly parsed statements, or
// don't return anything if error.
ATP_LOGIC_API boost::optional<std::list<ParseNodePtr>>
	parse(std::istream& in);


}  // namespace logic
}  // namespace atp


