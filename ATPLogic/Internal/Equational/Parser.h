#pragma once


/*

EquationalParser.h

A parser transforms text into a structured tree of expressions.
In this application, the parser performs no type checking. In
order to use the results of the parser it must first be converted
into a syntax tree (from the parse_statements tree). See the syntax tree code
for more information, or the parse_statements nodes file.

*/


#include <istream>
#include <list>
#include <string>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "ParseNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


// Parse an input stream of line-separated statements.
// Return a list of correctly parsed statements, or
// don't return anything if error.
ATP_LOGIC_API boost::optional<std::list<ParseNodePtr>>
	parse_statements(std::istream& in);


// Parse an input stream of line-separated definitions,
// where each line is just the symbol name and the symbol
// arity.
ATP_LOGIC_API boost::optional<std::list<
	std::pair<std::string, size_t>>>
	parse_definitions(std::istream& in);


}  // namespace equational
}  // namespace logic
}  // namespace atp


