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

		("search-settings,ss", po::value<std::string>(),
			"Path to the file containing the search settings")

		("prove,P", po::value<std::vector<std::string>>(),
			"Path to a file containing statements to prove, or write"
			" a statement with no spaces f(x)=y to try to prove")
	;

	// parse the arguments:
	po::variables_map vm;
	try
	{
		po::store(po::command_line_parser(argc, argv)
			.options(desc).run(), vm);
		po::notify(vm);
	}
	catch(std::exception& ex)
	{
		std::cout << "An error occurred while parsing the"
			<< " command line arguments: " << ex.what()
			<< std::endl;
		return -1;
	}

	return run_proof_application(vm);
}


int run_proof_application(const po::variables_map& vm)
{
	std::cout << "Starting up proof application..." << std::endl;

	ProofApplication app(std::cout);

	// check command line arguments

	if (!vm.count("prove"))
	{
		// this is not an error.
		std::cout << "No work to do; terminating." << std::endl;
		return 0;
	}
	if (!vm.count("context"))
	{
		std::cout << "Error: need to provide a context file."
			<< "Use --context <filename>" << std::endl;
		return -1;
	}
	if (!vm.count("search-settings"))
	{
		std::cout << "Error: need to provide a search settings file."
			<< "Use --search-settings <filename>" << std::endl;
		return -1;
	}

	std::string ctx_file = vm["context"].as<std::string>();

	// try to load context file
	if (!app.set_context_file(ctx_file))
	{
		return -1;
	}

	std::string ss_file = vm["search-settings"].as<std::string>();

	// try to load search file
	if (!app.set_search_file(ss_file))
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


