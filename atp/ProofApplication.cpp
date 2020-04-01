#include "ProofApplication.h"
#include <fstream>


bool ProofApplication::set_context_file(std::string path)
{
	std::ifstream in(path);

	if (!in)
		return false;

	return false;
}


bool ProofApplication::add_proof_task(std::string path_or_stmt)
{
	return false;
}


std::vector<std::pair<std::string, std::string>>
ProofApplication::run(std::ostream& out)
{
	return std::vector<std::pair<std::string, std::string>>();
}


