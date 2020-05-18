#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a function which parses the commands from std::cin
	for the server application's prompt.

*/


#include <functional>
#include <ATPCore.h>


typedef std::function<void()> CommandType;


// allows the commands to be created externally (and makes them able
// to be bound to the server application object, etc.)
struct CommandSet
{
	std::function<CommandType(int)> create_proof_cmd;
	std::function<CommandType()> create_help_cmd;
};


/**
\brief Input a command from std::cin and return it

\param cmd_set The constructor for all of the commands
*/
CommandType get_cmd(CommandSet& cmd_set);


