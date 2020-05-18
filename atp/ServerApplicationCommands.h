#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the commands which are executed from the server
	application's prompt.
*/


#include "ServerApplicationCmdParser.h"
#include <ATPCore.h>


void prove_command(int n, atp::core::ProcessManager&);
void help_command(atp::core::ProcessManager&);


