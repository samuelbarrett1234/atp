#include <vector>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <ATPLogic.h>
#include "ProofApplication.h"


namespace po = boost::program_options;


int run_proof_application(const po::variables_map& vm);


int main(int argc, const char* const argv[])
{
	// setting up program command line arguments

	po::options_description desc("Options:");

	desc.add_options()
		("context,ctx", po::value<std::string>(),
			"Path to the file containing the context for the proofs")

		("prove,P", po::value<std::vector<std::string>>(),
			"Path to a file containing statements to prove, or write"
			" a statement with no spaces f(x)=y to try to prove")

		("search_settings,ss", po::value<std::string>(),
			"Path to the file containing the settings for the search"
			" algorithms.")
	;

	// parse the arguments:
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
		.options(desc).run(), vm);
	po::notify(vm);

	// if we are asked to prove some statements...
	if (!vm.count("prove"))
	{
		return run_proof_application(vm);
	}
	else
	{
		std::cout << "No work given." << std::endl;
		return 0;
	}
}


int run_proof_application(const po::variables_map& vm)
{
	std::cout << "Starting up proof application..." << std::endl;

	ProofApplication app(std::cout);

	std::string ctx_file = vm["context"].as<std::string>();

	// try to load context file
	if (!app.set_context_file(ctx_file))
	{
		return -1;
	}
	
	// try to add all of the proof tasks:
	auto proof_tasks = vm["prove"].as<std::vector<std::string>>();
	for (auto task : proof_tasks)
	{
		if (!app.add_proof_task(task))
		{
			return -1;
		}
	}

	app.run();

	return 0;
}


