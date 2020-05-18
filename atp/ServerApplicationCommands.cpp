/**
\file

\author Samuel Barrett

*/


#include "ServerApplicationCommands.h"
#include <iostream>


void prove_command(int n, atp::core::ProcessManager&)
{
	std::cout << "Prove " << n << "!" << std::endl;
}


void help_command(atp::core::ProcessManager&)
{
	std::cout << "Usage:" << std::endl
		<< "`.prove N`\tCreate a new process to prove"
		" N statements." << std::endl <<
		"`.help`,`.h`\tDisplay help message."
		<< std::endl;
}


