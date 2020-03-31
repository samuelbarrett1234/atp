#include "Parser.h"
#include "Grammar.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_stream.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


boost::optional<std::list<ParseNodePtr>> parse_statements(std::istream& in)
{
	boost::spirit::istream_iterator begin(in);
	boost::spirit::istream_iterator end;

	std::list<ParseNodePtr> output;
	const bool ok = boost::spirit::qi::parse(
		begin, end, equational::StatementGrammar(),
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
	boost::spirit::istream_iterator begin(in);
	boost::spirit::istream_iterator end;

	std::map<std::string, size_t> _output;
	const bool ok = boost::spirit::qi::parse(
		begin, end, equational::DefinitionGrammar(),
		_output
	);

	// convert output to the kind we want:
	std::list<std::pair<std::string, size_t>> output;
	output.insert(output.end(), _output.begin(), _output.end());

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


