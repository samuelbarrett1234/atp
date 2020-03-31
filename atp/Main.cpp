#include <list>
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

		("prove,P", po::value<std::list<std::string>>(),
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
	if (!vm.count("prove") > 0)
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

	ProofApplication app;

	std::string ctx_file = vm["context"].as<std::string>();

	if (!boost::filesystem::is_regular_file(ctx_file))
	{
		std::cout << "Error: context file \"" << ctx_file <<
			"\" does not exist (as a file)." << std::endl;
		return -1;
	}

	// try to load context file
	if (!app.set_context_file(ctx_file))
	{
		std::cout << "Failed to load context file \"" << ctx_file
			<< "\"." << std::endl;
		return -1;
	}

	std::cout << "Loaded context file successfully." << std::endl;

	// try to add all of the proof tasks:
	auto proof_tasks = vm["prove"].as<std::list<std::string>>();
	for (auto task : proof_tasks)
	{
		if (!app.add_proof_task(task))
		{
			std::cout << "Failed to add proof task \"" << task <<
				"\". Are you sure that this either a correct file "
				"path, or a valid expression in the given context?"
				<< std::endl;

			return -1;
		}
	}

	std::cout << "Loaded all proof tasks successfully." << std::endl
		<< "Running proofs..." << std::endl;

	auto proof_results = app.run(std::cout);

	std::cout << "Done! Outputting proof results:" << std::endl;

	for (auto proof : proof_results)
	{
		std::cout << proof.first << "\t\t\t" << proof.second <<
			std::endl;
	}

	return 0;
}


