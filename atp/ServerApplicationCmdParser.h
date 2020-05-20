#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a function which parses the commands from std::cin
	for the server application's prompt.

*/


#include <functional>
#include <ATPCore.h>


// allows the commands to be called externally (and makes them able
// to be bound to the server application object, etc.)
// commands should return true iff success
struct CommandSet
{
	std::function<bool()> ls_cmd;
	std::function<bool(int)> set_threads_cmd;
	std::function<bool()> killall_cmd;
	std::function<bool()> help_cmd;
	std::function<bool()> exit_cmd;
};


/**
\brief Input a command from std::cin and execute it

\param cmd_set The set of all possible commands

\returns True iff success
*/
bool do_cmd(CommandSet& cmd_set);


