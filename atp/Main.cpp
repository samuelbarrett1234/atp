/**
\file

\author Samuel Barrett

\brief Entrypoint for `atp` command line application.

*/


#include <vector>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "ATP.h"
#include "ProofApplication.h"


namespace po = boost::program_options;


int run_proof_application(const po::variables_map& vm);
int run_create_db_from_schema(const po::variables_map& vm);


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

		("database,db", po::value<std::vector<std::string>>(),
			"Path to database configuration file.")

		("dbschema,dbsch", po::value<std::vector<std::string>>(),
			"Path to database schema to create a new database from.")
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

	if (vm.count("dbschema"))
	{
		// user wants to create a new database
		const int r = run_create_db_from_schema(vm);
		if (r != 0)
			return r;
	}

	if (vm.count("prove"))
	{
		// user wants to prove some statement(s)
		const int r = run_proof_application(vm);
		if (r != 0)
			return r;
	}

	std::cout << "Done. Exiting..." << std::endl;
	return 0;
}


int run_proof_application(const po::variables_map& vm)
{
	ProofApplication app(std::cout);

	// check command line arguments

	ATP_ASSERT(vm.count("prove"));

	if (!vm.count("context"))
	{
		std::cout << "Error: need to provide a context file."
			<< " Use --context <filename>" << std::endl;
		return -1;
	}
	if (!vm.count("search-settings"))
	{
		std::cout << "Error: need to provide a search settings file."
			<< " Use --search-settings <filename>" << std::endl;
		return -1;
	}

	const std::string ctx_file = vm["context"].as<std::string>();

	// try to load context file
	if (!app.set_context_file(ctx_file))
	{
		return -1;
	}

	// optional
	if (vm.count("database"))
	{
		std::string db_file = vm["database"].as<std::string>();

		// try to load database config file
		if (!app.set_db(db_file))
		{
			return -1;
		}
	}

	const std::string ss_file = vm["search-settings"].as<std::string>();

	// try to load search file
	if (!app.set_search_file(ss_file))
	{
		return -1;
	}
	
	// try to add all of the proof tasks:
	const auto proof_tasks = vm["prove"].as<std::vector<std::string>>();
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


int run_create_db_from_schema(const po::variables_map& vm)
{
	if (!vm.count("database"))
	{
		std::cout << "Error: need to provide path to output the "
			<< "database config file once done. Use --database "
			<< "<a-place-to-output-the-db-config-file>" << std::endl;
		return -1;
	}

	const std::string in_file = vm["dbschema"].as<std::string>();
	const std::string out_file = vm["database"].as<std::string>();

	std::ifstream in(in_file);
	std::ofstream out(out_file);

	if (!in)
	{
		std::cout << "Failed to open file \"" << in_file << "\"." <<
			std::endl;
		return -1;
	}

	if (!out)
	{
		std::cout << "Failed to create file \"" << out_file << "\"." <<
			std::endl;
		return -1;
	}

	auto result = atp::db::create_from_schema(in, out);

	if (!result)
	{
		std::cout << "Conversion operation failed! Check the JSON "
			<< "schema at \"" << in_file << "\" is in the correct "
			<< "format." << std::endl;

		out.close();

		// delete the output file in case anything was written
		boost::filesystem::remove(out_file);

		return -1;
	}

	// all files will automatically close here.
	return 0;
}


