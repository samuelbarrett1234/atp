/**
\file

\author Samuel Barrett

\brief Entrypoint for `atp` command line application.

*/


#include "ATP.h"  // first; it tells boost log to dynamic link
#include <memory>
#include <vector>
#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "Application.h"
#include "ServerApplication.h"


namespace po = boost::program_options;


// to allow us to use command line options which are pairs of
// integers
namespace std
{
istream& operator>>(istream& in, pair<size_t, size_t>& pair)
{
	char c;
	in >> pair.first >> c >> pair.second;
	return in;
}
}


int setup_logs(const po::variables_map& vm);
int load_db(std::unique_ptr<Application>& p_app,
	const po::variables_map& vm);
int load_ctx(std::unique_ptr<Application>& p_app,
	const po::variables_map& vm);
int load_ss(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm);
int add_proof(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm);
int add_wandering(std::unique_ptr<Application>& p_app,
	const po::variables_map& vm);
int add_hmm_conj(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm);
int add_hmm_train(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm);
int create_hmm_conj(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm);
int load_db_server(std::unique_ptr<ServerApplication>& p_app,
	const po::variables_map& vm);


int main(int argc, const char* const argv[])
{
	// setting up program command line arguments

	po::options_description desc("Options:");

	desc.add_options()
		("help,h", "Produce help message, then exit.")

		("verbose,v", "Print lots of extra information to console.")

		("surpress,s", "Only print errors and warnings.")

		("logfile,lf", po::value<std::string>(),
			"Change file to write log info to.")

		("nologfile,nlf", "Don't write any log files.")

		("serve", "Set up a server application, which continuously "
			"performs tasks, and is autonomous. Using this option "
			"ignores the `prove` option and options relating to "
			"automated conjecturing. *Requires a DB*, but *not* the "
			"context or search settings options.")

		("prove,P", po::value<std::vector<std::string>>(),
			"Usage: `--P <filename>` or `--P <stmt>` or `--P <N>` "
			"which launches a proof attempt at the statements in "
			"the file, or the given statement, or randomly picks "
			"N unproven statements and attempts to prove them, "
			"respectively. You can invoke this many times, each "
			"proof task can run in its own thread.")

		("wander,wdr", po::value<std::pair<size_t, size_t>>(),
			"Usage: `--wdr N,D` will launch a \"successor wandering"
			"\" process which generates N new true statements at "
			"depth D.")

		("hmm-conjecture,hmmc", po::value<std::vector<size_t>>(),
			"Generate a number of conjectures; use "
			"--hmm-conjecture N to generate N conjectures. Each "
			"invocation of this command will run on a separate "
			"thread.")

		("hmm-conjecture-train,hmmct", po::value<std::pair<size_t, size_t>>(),
			"`--hmmct N,M` will train the HMM conjecturer in the "
			"current context for N epochs on a dataset of size M."
			" Each epoch is a single pass over the dataset.")

		("create-hmm-conjecturer,chmmc", po::value<std::pair<size_t, size_t>>(),
			"Create a new HMM conjecturer for the current context. "
			"Use `--chmmc N,M` without spaces between N and M, where N"
			" is the number of hidden states, and M is the ID you'd"
			" like to give it.")

		("database,db", po::value<std::string>(),
			"Path to database configuration file (required).")

		("context,ctx", po::value<std::string>(),
			"Context name (see `model_contexts` database table) - "
			"required for all but the `serve` modes.")

		("search-settings,ss", po::value<std::string>(),
			"Search settings name (see `search_settings` database table) - "
			"required for all but the `serve` modes.")

		("nthreads,nt", po::value<size_t>(),
			"The number of threads to commit to the task at hand."
			" In `serve` mode you can change this later.")
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
	ATP_LOG(trace) << "**** STARTING UP APPLICATION ****";

	// create application:

	const size_t num_threads = vm.count("nthreads") ?
		vm.at("nthreads").as<size_t>() : 1;
	if (num_threads == 0)
	{
		ATP_LOG(error) << "Need at least one thread.";
		return -1;
	}

	if (vm.count("serve"))
	{
		std::unique_ptr<ServerApplication> p_app =
			std::make_unique<ServerApplication>(num_threads);

		if (int rc = load_db_server(p_app, vm))
		{
			return rc;
		}

		p_app->run();
	}
	else
	{
		std::unique_ptr<Application> p_app =
			std::make_unique<Application>(num_threads);

		if (int rc = load_db(p_app, vm))
			return rc;
		if (int rc = load_ctx(p_app, vm))
			return rc;
		if (int rc = load_ss(p_app, vm))
			return rc;
		if (int rc = add_proof(p_app, vm))
			return rc;
		if (int rc = add_wandering(p_app, vm))
			return rc;
		if (int rc = add_hmm_conj(p_app, vm))
			return rc;
		if (int rc = add_hmm_train(p_app, vm))
			return rc;
		if (int rc = create_hmm_conj(p_app, vm))
			return rc;

		p_app->run();
	}

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

	const int console_severity =
		// verbose means show all
		(vm.count("verbose") ? (int)severity_level::trace :
			// surpress means warnings only
			(vm.count("surpress") ? (int)severity_level::warning :
				// serve mode: don't show info
				(vm.count("serve") ? (int)severity_level::warning :
					// default: info
					(int)severity_level::info)));

	// always put everything into the file log
	const int file_severity = (int)severity_level::trace;

	// in server mode, overwrite the last log, unless the user has
	// specified a log file, in which case never overwrite it
	const auto open_mode = (vm.count("serve") && !vm.count("logfile")) ?
		std::ios_base::trunc : std::ios_base::app;

	boost::log::add_console_log(
		std::cout,
		boost::log::keywords::format = "%Severity% : %Message%")
		->set_filter(boost::log::trivial::severity
			>= console_severity);

	if (vm.count("logfile"))
	{
		boost::log::add_file_log(
			keywords::file_name = vm["logfile"].as<std::string>(),
			keywords::format =
			"%TimeStamp% [Thread %ThreadID%] %Severity% : %Message%",
			keywords::open_mode = open_mode
		)->set_filter(boost::log::trivial::severity
			>= file_severity);
	}
	else if (!vm.count("nologfile"))
	{
		boost::log::add_file_log(
			keywords::file_name = "atp_log_%N.txt",
			// rotate every 1Mb
			keywords::rotation_size = 1024 * 1024,
			keywords::format =
			"%TimeStamp% [Thread %ThreadID%] %Severity% : %Message%",
			keywords::open_mode = open_mode
		)->set_filter(boost::log::trivial::severity
			>= file_severity);
	}
	// else the user doesn't want a log file

	return 0;
}


int load_db(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
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
	if (!p_app->set_db(db_file))
	{
		return -1;
	}

	return 0;
}


int load_ctx(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
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
	if (!p_app->set_context_name(ctx_name))
	{
		return -1;
	}

	return 0;
}


int load_ss(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
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
	if (!p_app->set_search_name(ss_name))
	{
		return -1;
	}

	return 0;
}


int add_proof(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
{
	if (vm.count("prove"))
	{
		// try to add all of the proof tasks:
		const auto proof_tasks =
			vm["prove"].as<std::vector<std::string>>();
		for (auto task : proof_tasks)
		{
			try
			{
				// throws if not integer
				const size_t N = boost::lexical_cast<size_t>(task);

				if (!p_app->add_proof_task(N))
				{
					ATP_LOG(error)
						<< "Failed to load proof task: '"
						<< task << '\'';
					return -1;
				}
			}
			catch (boost::bad_lexical_cast&)
			{
				// handle as filename / statement input
				if (!p_app->add_proof_task(task))
				{
					ATP_LOG(error)
						<< "Failed to load proof task: '"
						<< task << '\'';
					return -1;
				}
			}
		}
	}

	return 0;
}


int add_wandering(std::unique_ptr<Application>& p_app,
	const po::variables_map& vm)
{
	if (vm.count("wander"))
	{
		const auto [N, D] = vm.at("wander").as<std::pair<size_t,
			size_t>>();
		
		if (!p_app->add_wanderer_task(N, D))
		{
			ATP_LOG(error) << "Failed to add wanderer task.";
			return -1;
		}
	}
	return 0;
}


int add_hmm_conj(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
{
	if (vm.count("hmm-conjecture"))
	{
		// try to add all the conjecture tasks:
		const auto conjecture_tasks =
			vm["hmm-conjecture"].as<std::vector<size_t>>();
		for (auto N : conjecture_tasks)
		{
			if (!p_app->add_hmm_conjecture_task(N))
			{
				ATP_LOG(error) << "Failed to launch conjecturer.";
				return -1;
			}
		}
	}

	return 0;
}


int add_hmm_train(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
{
	if (vm.count("hmm-conjecture-train"))
	{
		const auto params =
			vm["hmm-conjecture-train"].as<std::pair<size_t, size_t>>();
		const size_t epochs = params.first;
		const size_t dataset_size = params.second;

		if (!p_app->add_hmm_conj_train_task(epochs, dataset_size))
		{
			ATP_LOG(error) << "Failed to launch conjecturer "
				"training process.";
			return -1;
		}
	}

	return 0;
}


int create_hmm_conj(std::unique_ptr<Application>& p_app, 
	const po::variables_map& vm)
{
	if (vm.count("create-hmm-conjecturer"))
	{
		const auto tup =
			vm["create-hmm-conjecturer"].as<std::pair<size_t, size_t>>();
		const size_t num_hidden = tup.first;
		const size_t model_id = tup.second;

		if (num_hidden == 0)
		{
			ATP_CORE_LOG(error) <<
				"Cannot specify no hidden states.";
			return -1;
		}
		else
		{
			if (!p_app->create_hmm_conjecturer(num_hidden, model_id))
			{
				ATP_CORE_LOG(error) << "Failed to create HMM "
					"conjecturer.";
				return -1;
			}
		}
	}
	return 0;
}


int load_db_server(std::unique_ptr<ServerApplication>& p_app,
	const po::variables_map& vm)
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
	if (!p_app->set_db(db_file))
	{
		return -1;
	}

	return 0;
}


