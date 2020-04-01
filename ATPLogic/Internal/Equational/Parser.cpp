#include "Parser.h"
#include "Grammar.h"
#include <boost/spirit/include/qi.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


boost::optional<std::list<ParseNodePtr>> parse_statements(std::istream& in)
{
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

	if (!ok)
	{
		return boost::none;
	}
	// else succeeded
	else
	{
		return output;
	}
}


boost::optional<std::list<std::pair<std::string, size_t>>>
	parse_definitions(std::istream& in)
{
	QiParseIterator begin(in), end;

	// no input at all is not an error, so don't return boost::none,
	// so return an empty list instead:
	if (begin == end)
		return boost::make_optional(
			std::list<std::pair<std::string, size_t>>());

	std::list<std::pair<std::string, size_t>> output;
	const bool ok = boost::spirit::qi::phrase_parse(
		begin, end, DefinitionGrammar(), Skipper(),
		output
	);

	if (!ok)
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


