/**
\file

\author Samuel Barrett

*/


#include "ServerApplicationCmdParser.h"
#include <iostream>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/bind.hpp>
#include "ATP.h"


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
typedef std::string::iterator QiParseIterator;
typedef boost::spirit::ascii::blank_type SkipperType;


/**
\brief The grammar for the command line arguments
*/
struct CommandGrammar :
	public qi::grammar<QiParseIterator, bool,
	SkipperType>
{
	CommandGrammar(CommandSet& cmd_set) :
		CommandGrammar::base_type(start)
	{
		start = ls_cmd | help_cmd | exit_cmd |
			set_threads_cmd | set_scheduler_cmd;

		// it is important to parse qi::eoi (end of input) at the
		// end to prevent parsing only the start of an input and then
		// executing the command

		ls_cmd = (".ls" >> qi::eoi)
			[qi::_val = phx::bind(cmd_set.ls_cmd)];

		set_threads_cmd = (".set_threads" >> qi::int_ >> qi::eoi)
			[qi::_val = phx::bind(cmd_set.set_threads_cmd, qi::_1)];

		help_cmd = (qi::lit(".help") | ".h") >> qi::eoi
			[qi::_val = phx::bind(cmd_set.help_cmd)];

		exit_cmd = qi::lit(".exit") >> qi::eoi
			[qi::_val = phx::bind(cmd_set.exit_cmd)];

		set_scheduler_cmd = (qi::lit(".set_scheduler on") >> qi::eoi)
			[qi::_val = phx::bind(cmd_set.set_scheduler_cmd, true)] |
			(qi::lit(".set_scheduler off") >> qi::eoi)
			[qi::_val = phx::bind(cmd_set.set_scheduler_cmd, false)];
	}

	qi::rule<QiParseIterator, bool,
		SkipperType> start, ls_cmd,
		help_cmd, exit_cmd, set_threads_cmd,
		set_scheduler_cmd;
};


bool do_cmd(CommandSet& cmd_set)
{
	ATP_PRECOND((bool)cmd_set.ls_cmd);
	ATP_PRECOND((bool)cmd_set.help_cmd);
	ATP_PRECOND((bool)cmd_set.exit_cmd);
	ATP_PRECOND((bool)cmd_set.set_threads_cmd);
	ATP_PRECOND((bool)cmd_set.set_scheduler_cmd);

	// do commands line by line
	std::string line;
	std::getline(std::cin, line);

	// don't throw error for empty line:
	if (line == "")
		return true;

	bool cmd_succ = false;
	const bool ok = qi::phrase_parse(
		line.begin(), line.end(), CommandGrammar(cmd_set),
		SkipperType(), cmd_succ
	);

	return ok && cmd_succ;
}


