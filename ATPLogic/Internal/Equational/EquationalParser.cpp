#include "EquationalParser.h"
#include "EquationalGrammar.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_stream.hpp>


namespace atp
{
namespace logic
{


boost::optional<std::list<ParseNodePtr>> parse_statements(std::istream& in)
{
	boost::spirit::istream_iterator begin(in);
	boost::spirit::istream_iterator end;

	std::list<ParseNodePtr> output;
	boost::spirit::qi::phrase_parse(
		begin, end, grammar::StatementGrammar(),
		grammar::Skipper(),  // custom skipper for comments etc
		output
	);

	// failed if we didn't get to the end
	if (begin != end)
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

	std::list<std::pair<std::string, size_t>> output;
	boost::spirit::qi::phrase_parse(
		begin, end, grammar::DefinitionGrammar(),
		grammar::Skipper(),  // custom skipper for comments etc
		output
	);

	// failed if we didn't get to the end
	if (begin != end)
	{
		return boost::none;
	}
	// else succeeded
	else
	{
		return output;
	}
}


}  // namespace logic
}  // namespace atp


