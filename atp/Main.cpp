/**
\file

\author Samuel Barrett

\brief Entrypoint for `atp` command line application.

*/


#include <memory>
#include <vector>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "ATP.h"
#include "Application.h"


namespace po = boost::program_options;


std::unique_ptr<Application> g_app;


int load_db(const po::variables_map& vm);
int load_ctx(const po::variables_map& vm);
int load_ss(const po::variables_map& vm);
int add_proof(const po::variables_map& vm);


int main(int argc, const char* const argv[])
{
	// setting up program command line arguments

	po::options_description desc("Options:");

	desc.add_options()
		("help,h", "produce help message.")

		("prove,P", po::value<std::vector<std::string>>(),
			"Path to a file containing statements to prove, or write"
			" a statement with no spaces f(x)=y to try to prove")

		("database,db", po::value<std::string>(),
			"Path to database file")

		("context,ctx", po::value<std::string>(),
			"Context name (see `model_contexts` database table)")

		("search-settings,ss", po::value<std::string>(),
			"Search settings name (see `search_settings` database table)")

		("nthreads,nt", po::value<size_t>(),
			"The number of threads to commit to the task at hand.")
	;

	// parse the arguments:
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch(std::exception& ex)
	{
		std::cout << "An error occurred while parsing the"
			<< " command line arguments: " << ex.what()
			<< std::endl;
		return -1;
	}

	// if user wants help!
	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 0;
	}

	// create application:

	const size_t num_threads = vm.count("nthreads") ?
		vm.at("nthreads").as<size_t>() : 1;
	if (num_threads == 0)
	{
		std::cout << "Error: need at least one thread." << std::endl;
		return -1;
	}
	g_app = std::make_unique<Application>(std::cout,
		num_threads);

	if (int rc = load_db(vm))
		return rc;
	if (int rc = load_ctx(vm))
		return rc;
	if (int rc = load_ss(vm))
		return rc;
	if (int rc = add_proof(vm))
		return rc;

	g_app->run();

	std::cout << "Done. Exiting..." << std::endl;
	return 0;
}


int load_db(const po::variables_map& vm)
{
	if (!vm.count("database"))
	{
		std::cout << "Error: need to provide a database file."
			<< " Use --database <filename>" << std::endl;
		return -1;
	}

	const std::string db_file = vm["database"].as<std::string>();

	// try to load database config file
	if (!g_app->set_db(db_file))
	{
		return -1;
	}

	return 0;
}


int load_ctx(const po::variables_map& vm)
{
	if (!vm.count("context"))
	{
		std::cout << "Error: need to provide a context name."
			<< " Use --context <name>" << std::endl;
		return -1;
	}

	const std::string ctx_name = vm["context"].as<std::string>();

	// try to load context file
	if (!g_app->set_context_name(ctx_name))
	{
		return -1;
	}

	return 0;
}


int load_ss(const po::variables_map& vm)
{
	if (!vm.count("search-settings"))
	{
		std::cout << "Error: need to provide a search settings name."
			<< " Use --search-settings <name>" << std::endl;
		return -1;
	}

	const std::string ss_name = vm["search-settings"].as<std::string>();

	// try to load search file
	if (!g_app->set_search_name(ss_name))
	{
		return -1;
	}

	return 0;
}


int add_proof(const po::variables_map& vm)
{
	// try to add all of the proof tasks:
	const auto proof_tasks = vm["prove"].as<std::vector<std::string>>();
	for (auto task : proof_tasks)
	{
		if (!g_app->add_proof_task(task))
		{
			return -1;
		}
	}

	return 0;
}


