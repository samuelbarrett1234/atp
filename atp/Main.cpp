#include <boost/program_options.hpp>
#include <ATPLogic.h>
#include <vector>


namespace po = boost::program_options;


int main(int argc, const char* const argv[])
{
	// where we will store some results of command line arguments:
	size_t num_proof_threads = 0, num_db_threads = 0,
		max_proof_depth = 0;

	// setting up program command line arguments

	po::options_description desc("Options:");

	desc.add_options()
		("client,c", "Run the application in client mode"
			" (default)")

		("server,S", "Run the application in server mode"
			" (not implemented)")

		("language,l", po::value<atp::logic::LangType>(),
			"Set the language type (affects which definition"
			" files you can use.)")

		("def,d", po::value<std::vector<std::string>>(),
			"The filename of a definition file, written in"
			" the chosen language settings.")

		("solve,s", po::value<std::vector<std::string>>(),
			"The filename of a list of statements which need"
			" to be attempted to be proven.")

		("db,D", po::value<std::string>(),
			"The filename of the database to use.")

		("num_proof_threads,npt",
			po::value<size_t>(&num_proof_threads)->default_value(1),
			"The number of threads to use for proof processes.")

		("num_db_threads,ndt",
			po::value<size_t>(&num_db_threads)->default_value(1),
			"The number of threads used for database management,"
			" which includes syncing with other machines if"
			" applicable.")

		("max_proof_depth,md",
			po::value<size_t>(&max_proof_depth)->default_value(100),
			"The maximum number of steps that a proof can use.")
		;

	// all positional arguments are definition files, by default
	po::positional_options_description pos;
	pos.add("def", -1);

	// parse the arguments:
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv)
		.options(desc).positional(pos).run(), vm);
	po::notify(vm);

	// TODO: handle them all!
	// Use: vm["name"].as<Type>()

	return 0;
}


