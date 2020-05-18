#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a function which parses the commands from std::cin
	for the server application's prompt.

*/


#include <functional>
#include <ATPCore.h>


typedef std::function<void(atp::core::ProcessManager&)> CommandType;


/**
\brief Input a command from std::cin and return it
*/
CommandType get_cmd();


