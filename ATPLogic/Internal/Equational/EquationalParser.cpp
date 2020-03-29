#include "EquationalParser.h"
#include "EquationalGrammar.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_stream.hpp>


namespace atp
{
namespace logic
{


boost::optional<std::list<ParseNodePtr>> parse(std::istream& in)
{
	boost::spirit::istream_iterator begin(in);
	boost::spirit::istream_iterator end;

	std::list<ParseNodePtr> output;
	boost::spirit::qi::phrase_parse(
		begin, end, grammar::Parser(),
		grammar::Skipper(),  // custom skipper for comments etc
		output
	);

	// failed if we didn't get to the end
	if (begin != end)
	{
		return boost::optional<std::list<ParseNodePtr>>();
	}
	// else succeeded
	else
	{
		return output;
	}
}


}  // namespace logic
}  // namespace atp


