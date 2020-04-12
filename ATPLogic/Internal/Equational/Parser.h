#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the functions for turning a string into a parse tree
    of expressions (note that parse trees and syntax trees are
	different).

\details A parser transforms text into a structured tree of
    expressions. In this application, the parser performs no type
	checking. In order to use the results of the parser it must first
	be converted into a syntax tree (from the parse tree). See the
	syntax tree code for more information.

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


/**
\brief Read a stream of line separated statements, and return the
    list of their parse trees, or None if an error occurred with
	any of them.

\note Comments are done using the hashtag #
*/
ATP_LOGIC_API boost::optional<std::list<ParseNodePtr>>
	parse_statements(std::istream& in);


}  // namespace equational
}  // namespace logic
}  // namespace atp


