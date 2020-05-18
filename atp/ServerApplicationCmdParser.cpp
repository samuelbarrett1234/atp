/**
\file

\author Samuel Barrett

*/


#include "ServerApplicationCmdParser.h"
#include <iostream>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/bind.hpp>


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
typedef boost::spirit::istream_iterator QiParseIterator;
typedef boost::spirit::ascii::space_type SkipperType;


/**
\brief The grammar for the command line arguments
*/
struct CommandGrammar :
	public qi::grammar<QiParseIterator,
	CommandType, SkipperType>
{
	CommandGrammar(CommandSet& cmd_set) :
		CommandGrammar::base_type(start)
	{
		start = prove_cmd | help_cmd;

		prove_cmd = ".prove " >> qi::int_
			[cmd_set.create_proof_cmd];

		help_cmd = (qi::lit(".help ") | ".h")
			[cmd_set.create_help_cmd];
	}

	qi::rule<QiParseIterator, CommandType,
		SkipperType> start, prove_cmd, help_cmd;
};


CommandType get_cmd(CommandSet& cmd_set)
{
	QiParseIterator begin(std::cin), end;
	CommandType cmd;

	const bool ok = qi::phrase_parse(
		begin, end, CommandGrammar(cmd_set), SkipperType(),
		cmd
	);

	if (!ok)
		return CommandType();
	else
		return cmd;
}
