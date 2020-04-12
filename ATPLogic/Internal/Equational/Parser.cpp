/**

\file

\author Samuel Barrett

\see Grammar.h Grammar.cpp

*/


#include "Parser.h"
#include <boost/spirit/include/qi.hpp>
#include "Grammar.h"


namespace atp
{
namespace logic
{
namespace equational
{


boost::optional<std::list<ParseNodePtr>> parse_statements(
	std::istream& in)
{
	in >> std::noskipws;

	QiParseIterator begin(in), end;

	// no input at all is not an error, so don't return boost::none,
	// so return an empty list instead:
	if (begin == end)
		return boost::make_optional(std::list<ParseNodePtr>());

	std::list<ParseNodePtr> output;
	const bool ok = boost::spirit::qi::phrase_parse(
		begin, end, StatementGrammar(), Skipper(),
		output
	);

	if (!ok || begin != end)
	{
		return boost::none;
	}
	// else succeeded
	else
	{
		return output;
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


