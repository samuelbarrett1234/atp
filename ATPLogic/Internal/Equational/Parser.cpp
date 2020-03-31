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

	std::list<ParseNodePtr> output;
	const bool ok = boost::spirit::qi::phrase_parse(
		begin, end, StatementGrammar(), Skipper(),
		output
	);

	// failed if we didn't get to the end
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


boost::optional<std::list<std::pair<std::string, size_t>>>
	parse_definitions(std::istream& in)
{
	QiParseIterator begin(in), end;

	std::list<std::pair<std::string, size_t>> output;
	const bool ok = boost::spirit::qi::phrase_parse(
		begin, end, DefinitionGrammar(), Skipper(),
		output
	);

	// failed if we didn't get to the end
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


