/**
\file

\author Samuel Barrett

\brief Entrypoint for ATPSiteTools exe

*/


#define BOOST_LOG_DYN_LINK
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/algorithm/string/join.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>


#define ATP_LOG(level) BOOST_LOG_TRIVIAL(level)


namespace po = boost::program_options;


void setup_logging();


/**
\brief Check whether the given statement is valid

\param stmts The statements to check, in text format

\param ctx The name of the model context

\param db The path to the database file

\returns 0 if good statement, 1 if bad statement, -1 if failed to load
*/
int check_stmts(std::vector<std::string> stmts, std::string ctx,
	std::string db);


/**
\brief Normalise a list of statements, and output them line by line
	to the console.

\param stmts The statements to normalise, in text format

\param ctx The name of the model context

\param db The path to the database file

\returns 0 if success, 1 if bad statements, -1 if failed to load
*/
int norm_stmts(std::vector<std::string> stmts, std::string ctx,
	std::string db);


int main(int argc, const char* const argv[])
{

	po::options_description desc("Options:");

	desc.add_options()
		("help,h", "Produce help message, then exit.")

		("db", po::value<std::string>(),
			"Path to database file.")

		("ctx", po::value<std::string>(),
			"Name of model context to use.")

		("chk-stmt", po::value<std::vector<std::string>>(),
			"`--chk-stmt S` will check whether the "
			"statement `S` is valid in the given context."
			" You can pass this option multiple times.")

		("norm", po::value<std::vector<std::string>>(),
			"`--norm S` will output a normalised version of the "
			"statement to the console."
			" You can pass this option multiple times.")
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

			return check_stmts(vm["chk-stmt"].as<std::vector<std::string>>(),
				vm["ctx"].as<std::string>(), vm["db"].as<std::string>());
		}

		if (vm.count("norm"))
		{
			if (!vm.count("db") || !vm.count("ctx"))
			{
				ATP_LOG(error) <<
					"Need --db and --ctx to normalise a statement.";
				return -1;
			}

			return norm_stmts(vm["norm"].as<std::vector<std::string>>(),
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


int check_stmts(std::vector<std::string> stmts, std::string ctx,
	std::string db)
{
	ATP_LOG(trace) << "Beginning check operation on "
		"statements " << boost::algorithm::join(stmts, "\n");

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

	// get line separated statements
	std::stringstream stmt_strstream;
	for (const auto& stmt : stmts)
		stmt_strstream << stmt << "\n";

	auto p_stmt = p_lang->deserialise_stmts(stmt_strstream,
		atp::logic::StmtFormat::TEXT, *p_ctx);

	const int chk_result = (p_stmt != nullptr) ? 0 : 1;

	ATP_LOG(info) << "Check result: " << chk_result;
	
	return chk_result;
}


int norm_stmts(std::vector<std::string> stmts, std::string ctx,
	std::string db)
{
	ATP_LOG(trace) << "Beginning normalisation operation on "
		"statements " << boost::algorithm::join(stmts, "\n");

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

	// get line separated statements
	std::stringstream stmt_strstream;
	for (const auto& stmt : stmts)
		stmt_strstream << stmt << "\n";

	auto p_stmts = p_lang->deserialise_stmts(stmt_strstream,
		atp::logic::StmtFormat::TEXT, *p_ctx);

	if (p_stmts == nullptr)
	{
		ATP_LOG(warning) << "Bad statements inputted by user for "
			"normalisation. They were: \"" << stmt_strstream.str()
			<< '"';
		return 1;
	}

	// normalise then write results to output
	p_stmts = p_lang->normalise(p_stmts);
	for (size_t i = 0; i < p_stmts->size(); ++i)
	{
		const auto stmt_str = p_stmts->at(i).to_str();
		ATP_LOG(info) << "Normalisation result: \"" << stmt_str;
		std::cout << stmt_str << std::endl;
	}

	return 0;
}


