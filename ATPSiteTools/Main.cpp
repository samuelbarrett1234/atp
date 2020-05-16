/**
\file

\author Samuel Barrett

\brief Entrypoint for ATPSiteTools exe

*/


#define BOOST_LOG_DYN_LINK
#include <string>
#include <fstream>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>


#define ATP_LOG(level) BOOST_LOG_TRIVIAL(level)


namespace po = boost::program_options;


void setup_logging();


/**
\brief Check whether the given statement is valid

\param stmt The statement in text format

\param ctx The name of the model context

\param db The path to the database file

\returns 0 if good statement, 1 if bad statement, -1 if failed to load
*/
int check_stmt(std::string stmt, std::string ctx, std::string db);


int main(int argc, const char* const argv[])
{

	po::options_description desc("Options:");

	desc.add_options()
		("help,h", "Produce help message, then exit.")

		("db", po::value<std::string>(),
			"Path to database file.")

		("ctx", po::value<std::string>(),
			"Name of model context to use.")

		("chk-stmt", po::value<std::string>(),
			"`--chk-stmt S` will check whether the "
			"statement `S` is valid in the given context.")
		;

	// parse the arguments:
	po::variables_map vm;
	try
	{
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// if user wants help!
		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}

		setup_logging();

		if (vm.count("chk-stmt"))
		{
			if (!vm.count("db") || !vm.count("ctx"))
			{
				ATP_LOG(error) <<
					"Need --db and --ctx to check a statement.";
				return -1;
			}

			return check_stmt(vm["chk-stmt"].as<std::string>(),
				vm["ctx"].as<std::string>(), vm["db"].as<std::string>());
		}

		return 0;
	}
	catch (std::exception& ex)
	{
		ATP_LOG(error)
			<< "An error occurred while parsing the"
			<< " command line arguments: " << ex.what();
		return -1;
	}
}


void setup_logging()
{
	namespace keywords = boost::log::keywords;
	using boost::log::trivial::severity_level;

	boost::log::add_common_attributes();

	boost::log::add_file_log(
		keywords::file_name = "atp_site_tools_log_%N.txt",
		// rotate every 10Mb
		keywords::rotation_size = 10 * 1024 * 1024,
		keywords::format =
		"%TimeStamp% [Thread %ThreadID%] %Severity% : %Message%",
		keywords::open_mode = std::ios_base::app
	);
}


int check_stmt(std::string stmt, std::string ctx, std::string db)
{
	auto p_db = atp::db::load_from_file(db,
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (p_db == nullptr)
	{
		ATP_LOG(fatal) << "Could not load database from file \""
			<< db << '"';
		return -1;
	}

	auto maybe_ctx_fname = p_db->model_context_filename(ctx);

	p_db.reset();  // no longer need this

	if (!maybe_ctx_fname.has_value())
	{
		ATP_LOG(error) << "Bad context name \""
			<< ctx << '"';
		return -1;
	}

	std::ifstream fin(*maybe_ctx_fname);

	if (!fin)
	{
		ATP_LOG(error) << "Bad context filename \""
			<< *maybe_ctx_fname <<
			"\" occurred under context name \""
			<< ctx << '"';
		return -1;
	}

	auto p_lang = atp::logic::create_language(
		atp::logic::LangType::EQUATIONAL_LOGIC);

	auto p_ctx = p_lang->try_create_context(fin);

	if (p_ctx == nullptr)
	{
		ATP_LOG(error) << "Bad context file contents in \""
			<< *maybe_ctx_fname << '"';
		return -1;
	}

	std::stringstream stmt_strstream(stmt);

	auto p_stmt = p_lang->deserialise_stmts(stmt_strstream,
		atp::logic::StmtFormat::TEXT, *p_ctx);

	return (p_stmt != nullptr) ? 0 : 1;
}


