/**
\file

\author Samuel Barrett

*/


#include "ServerApplicationCmdParser.h"
#include <iostream>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/bind.hpp>
#include "ServerApplicationCommands.h"


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
typedef boost::spirit::istream_iterator QiParseIterator;
typedef boost::spirit::ascii::space_type SkipperType;


CommandType create_prove_cmd(int n)
{
	return boost::bind(&prove_command, n, _1);
}


CommandType create_help_cmd()
{
	return boost::bind(&help_command, _1);
}


/**
\brief The grammar for the command line arguments
*/
struct CommandGrammar :
	public qi::grammar<QiParseIterator,
	CommandType, SkipperType>
{
	CommandGrammar() :
		CommandGrammar::base_type(start)
	{
		start = prove_cmd | help_cmd;

		prove_cmd = ".prove " >> qi::int_
			[&create_prove_cmd];

		help_cmd = (qi::lit(".help ") | ".h")
			[&create_help_cmd];
	}

	qi::rule<QiParseIterator, CommandType,
		SkipperType> start, prove_cmd, help_cmd;
};


CommandType get_cmd()
{
	QiParseIterator begin(std::cin), end;
	CommandType cmd;

	const bool ok = qi::phrase_parse(
		begin, end, CommandGrammar(), SkipperType(),
		cmd
	);

	if (!ok)
		return CommandType();
	else
		return cmd;
}
