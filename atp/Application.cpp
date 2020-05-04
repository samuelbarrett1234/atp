/**

\file

\author Samuel Barrett

*/


#include "Application.h"
#include <sstream>
#include <fstream>
#include <list>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <ATPSearch.h>
#include "ATP.h"
#include "ProofProcess.h"


Application::Application(std::ostream& out,
	size_t num_threads) :
	m_out(out),
	m_num_threads(num_threads)
{
	ATP_PRECOND(num_threads > 0);
}


bool Application::set_db(const std::string& path)
{
	m_db = atp::db::load_from_file(path,
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (m_db == nullptr)
	{
		m_out << "Failed to create the database! Check the database "
			<< "file at \"" << path << "\" exists and is correct."
			<< std::endl;
		return false;
	}

	return true;
}


bool Application::set_context_name(const std::string& name)
{
	ATP_PRECOND(m_db != nullptr);

	const auto maybe_path = m_db->model_context_filename(name);
	const auto maybe_id = m_db->model_context_id(name);

	if (!maybe_path || !maybe_id)
	{
		m_out << "Error: context name \"" << name << "\" could not"
			<< " be obtained from the database. Check spelling and "
			<< "the database's `model_contexts` table." << std::endl;

		return false;
	}

	m_ctx_id = *maybe_id;

	const auto path = *maybe_path;

	if (!boost::filesystem::is_regular_file(path))
	{
		m_out << "Error: context file \"" << path <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	std::ifstream ctx_in(path);

	if (!ctx_in)
	{
		m_out << "There was a problem opening the file \"" <<
			path << "\"." << std::endl;

		return false;
	}

	// todo: allow the user to specify this
	m_lang = atp::logic::create_language(
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (m_lang == nullptr)
	{
		m_out << "Failed to create language object." << std::endl;
		return false;
	}

	m_ctx = m_lang->try_create_context(ctx_in);

	// if we fail here, it's because the JSON parsing failed
	if (!m_ctx)
	{
		m_out << "There was a problem loading the file \"" <<
			path << "\". Check for JSON syntax mistakes." << std::endl;

		return false;
	}

	m_out << "Successfully loaded the context file \"" << 
		m_ctx->context_name() << "\"." << std::endl;

	// here is a good place to do the check:
	check_axioms_in_db();

	return true;  // success
}


bool Application::set_search_name(const std::string& name)
{
	ATP_PRECOND(m_ctx != nullptr);
	ATP_PRECOND(m_db != nullptr);

	const auto maybe_path = m_db->search_settings_filename(name);
	const auto maybe_id = m_db->search_settings_id(name);

	if (!maybe_path || !maybe_id)
	{
		m_out << "Error: search settings name \"" << name
			<< "\" could not be obtained from the database. Check "
			"spelling and the database's `search_settings` table."
			<< std::endl;
		return false;
	}

	m_ss_id = *maybe_id;

	const auto path = *maybe_path;

	if (!boost::filesystem::is_regular_file(path))
	{
		m_out << "Error: search settings file \"" << path <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	std::ifstream in(path);

	if (!in)
	{
		m_out << "There was a problem opening the file \"" <<
			path << "\"." << std::endl;

		return false;
	}

	if (!atp::search::load_search_settings(
		in, &m_search_settings))
	{
		m_out << "There was a problem parsing the file \"" <<
			path << "\"." << std::endl;
		return false;
	}

	// now extract information from the `settings` object:

	if (!m_search_settings.create_solver)
	{
		// use the default solver

		m_search_settings.create_solver = boost::bind(
			&atp::search::create_default_solver, _1,
			atp::logic::iter_settings::DEFAULT);
	}

	m_out << "Successfully loaded search settings \"" <<
		m_search_settings.name << "\"" << std::endl;

	return true;  // success
}


bool Application::add_proof_task(std::string path_or_stmt)
{
	if (!m_lang || !m_ctx || !m_db)
		return false;  // not properly loaded

	atp::logic::StatementArrayPtr p_stmts;

	if (boost::filesystem::is_regular_file(path_or_stmt))
	{
		std::ifstream in(path_or_stmt);

		if (!in)
		{
			m_out << "Could not open the file \""
				<< path_or_stmt << std::endl;
			return false;
		}

		p_stmts = m_lang->deserialise_stmts(in,
			atp::logic::StmtFormat::TEXT,
			*m_ctx);
	}
	else
	{
		std::stringstream s(path_or_stmt);

		p_stmts = m_lang->deserialise_stmts(s,
			atp::logic::StmtFormat::TEXT,
			*m_ctx);
	}

	if (!p_stmts)
	{
		m_out << "Failed to load statements: \"" <<
			path_or_stmt << "\". Check syntax and types."
			<< std::endl;

		return false;
	}

	// add a proof process
	m_proc_mgr.add(std::make_unique<ProofProcess>(m_lang,
		m_ctx_id, m_ss_id, m_ctx, m_db, m_search_settings,
		std::move(p_stmts)));

	return true;
}


void Application::run()
{
	std::list<std::thread> threads;

	for (size_t i = 0; i < m_num_threads - 1; ++i)
	{
		threads.emplace_back(boost::bind(
			&ProcessManager::commit_thread, boost::ref(m_proc_mgr),
			boost::ref(m_out)));
	}

	m_proc_mgr.commit_thread(m_out);

	for (auto& th : threads)
		th.join();
}


void Application::check_axioms_in_db()
{
	ATP_PRECOND(m_db != nullptr);
	ATP_PRECOND(m_ctx != nullptr);

	auto _p_bder = m_db->create_query_builder(
		atp::db::QueryBuilderType::CHECK_AXIOMS_IN_DB);
	auto p_bder = dynamic_cast<atp::db::ICheckAxInDbQryBder*>(
		_p_bder.get());

	ATP_ASSERT(p_bder != nullptr);
	p_bder->set_ctx(m_ctx_id, m_ctx)
		->set_lang(m_lang);

	auto p_trans = m_db->begin_transaction(p_bder->build());

	if (p_trans == nullptr)
	{
		m_out << "ERROR! Failed to create the query which checks "
			<< "that all the axioms are loaded into the database."
			<< std::endl;
		return;
	}

	// run the transaction (for at most N steps, but it should be
	// reasonably done in N steps)
	static const size_t N = 100;
	for (size_t i = 0; i < N &&
		p_trans->state() == atp::db::TransactionState::RUNNING;
		++i)
		p_trans->step();

	if (p_trans->state() != atp::db::TransactionState::COMPLETED)
	{
		m_out << "ERROR! Failed to run query which checks that all"
			<< " the axioms are loaded into the database."
			<< std::endl;
		return;
	}
	// else we're all good
}


