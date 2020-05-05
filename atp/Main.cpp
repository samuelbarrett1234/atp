/**
\file

\author Samuel Barrett

\brief Entrypoint for `atp` command line application.

*/


#include "ATP.h"
#include <memory>
#include <vector>
#include <iostream>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "Application.h"


namespace po = boost::program_options;


std::unique_ptr<Application> g_app;


int setup_logs(const po::variables_map& vm);
int load_db(const po::variables_map& vm);
int load_ctx(const po::variables_map& vm);
int load_ss(const po::variables_map& vm);
int add_proof(const po::variables_map& vm);


int main(int argc, const char* const argv[])
{
	// setting up program command line arguments

	po::options_description desc("Options:");

	desc.add_options()
		("help,h", "Produce help message, then exit.")

		("verbose,v", "Print extra (trace) information to console.")

		("surpress,s", "Only print errors and warnings.")

		("logfile,lf", po::value<std::string>(),
			"Change file to write log info to.")

		("nologfile,nlf", "Don't write any log files.")

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
		ATP_LOG(error)
			<< "An error occurred while parsing the"
			<< " command line arguments: " << ex.what();
		return -1;
	}

	// if user wants help!
	if (vm.count("help"))
	{
		// don't use logging here, it's not set up yet
		std::cout << desc << std::endl;
		return 0;
	}

	// handle logging before creating application

	setup_logs(vm);

	// create application:

	const size_t num_threads = vm.count("nthreads") ?
		vm.at("nthreads").as<size_t>() : 1;
	if (num_threads == 0)
	{
		ATP_LOG(error) << "Need at least one thread.";
		return -1;
	}
	g_app = std::make_unique<Application>(num_threads);

	if (int rc = load_db(vm))
		return rc;
	if (int rc = load_ctx(vm))
		return rc;
	if (int rc = load_ss(vm))
		return rc;
	if (int rc = add_proof(vm))
		return rc;

	g_app->run();

	ATP_LOG(trace) << "Done. Exiting...";
	return 0;
}


int setup_logs(const po::variables_map& vm)
{
	namespace keywords = boost::log::keywords;
	using boost::log::trivial::severity_level;

	if (vm.count("verbose") && vm.count("surpress"))
	{
		std::cout << "Error: cannot use both --verbose and "
			"--surpress." << std::endl;
		return -1;
	}

	if (vm.count("logfile") && vm.count("nologfile"))
	{
		std::cout << "Error: cannot use both --verbose and "
			"--surpress." << std::endl;
		return -1;
	}

	boost::log::add_common_attributes();

	boost::log::add_console_log(
		std::cout,
		boost::log::keywords::format = "%Severity% : %Message%",

		// determine severity level from --verbose and --surpress
		keywords::severity = (vm.count("verbose") ?
			severity_level::trace : (vm.count("surpress") ?
				severity_level::warning : severity_level::info)));

	if (vm.count("logfile"))
	{
		boost::log::add_file_log(
			keywords::file_name = vm["logfile"].as<std::string>(),
			keywords::severity = severity_level::trace,
			keywords::format = "%TimeStamp% [Thread %ThreadID%] %Severity% : %Message%"
		);
	}
	else if (!vm.count("nologfile"))
	{
		boost::log::add_file_log(
			keywords::file_name = "atp_log_%N.txt",
			keywords::severity = severity_level::trace,
			// rotate every 10Mb
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::format = "%TimeStamp% [Thread %ThreadID%] %Severity% : %Message%"
		);
	}
	// else the user doesn't want a log file

	return 0;
}


int load_db(const po::variables_map& vm)
{
	if (!vm.count("database"))
	{
		ATP_LOG(error)
			<< "Need to provide a database file."
			<< " Use --database <filename>";
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
		ATP_LOG(error)
			<< "Need to provide a context name."
			<< " Use --context <name>";
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
		ATP_LOG(error)
			<< "Need to provide a search settings name."
			<< " Use --search-settings <name>";
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
			ATP_LOG(error)
				<< "Failed to load proof task: '" << task << '\'';
			return -1;
		}
	}

	return 0;
}


