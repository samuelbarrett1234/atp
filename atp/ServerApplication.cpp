/**
\file

\author Samuel Barrett
*/


#include "ServerApplication.h"
#include <iostream>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>
#include "ServerApplicationCmdParser.h"


ServerApplication::ServerApplication(size_t num_threads) :
	m_done(false)
{
	// initialise worker threads:
	ATP_PRECOND(num_threads > 0);
	for (size_t i = 0; i < num_threads; ++i)
	{
		m_workers.emplace_back([this]()
			{ worker_thread_func(); });
	}
}


bool ServerApplication::set_db(const std::string& path)
{
	m_db = atp::db::load_from_file(path,
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (m_db == nullptr)
	{
		ATP_LOG(error) << "Failed to create the database! "
			<< "Check the database file at \"" << path
			<< "\" exists and is correct.";
		return false;
	}

	ATP_LOG(trace) << "Successfully loaded database file at \""
		<< path << '"';

	return true;
}


void ServerApplication::run()
{
	ATP_PRECOND(!m_done);
	ATP_PRECOND(m_db != nullptr);

	// the parser will handle the skipping
	std::cin >> std::noskipws;

	// set up command creators:
	CommandSet cmd_set;
	cmd_set.proof_cmd = boost::bind(
		&ServerApplication::prove_cmd, this, _1);
	cmd_set.help_cmd = boost::bind(&ServerApplication::help_cmd,
		this);
	cmd_set.exit_cmd = boost::bind(
		&ServerApplication::exit_cmd, this);

	// set up initial tasks
	initialise_tasks();

	while (!is_done())
	{
		// accept user command input

		std::cout << "> ";

		const bool ok = do_cmd(cmd_set);

		if (!ok)
		{
			std::cout << "Error parsing / executing command. "
				"Try `.help` for help." << std::endl;
		}
	}

	// workers are automatically joined by the exit command
}


void ServerApplication::worker_thread_func()
{
	using namespace std::chrono_literals;

	while (!is_done())
	{
		// note that, most of the time, this will block for ages,
		// but on the off-chance that the process manager has no
		// work left to do, this will exit, but we need to make
		// sure we keep going until is_done() returns true
		m_proc_mgr.commit_thread();

		// sleep to prevent spinning when we have no work to do
		std::this_thread::sleep_for(100ms);
	}
}


bool ServerApplication::is_done() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_mutex);

	return m_done;
}


void ServerApplication::initialise_tasks()
{
	ATP_LOG(info) << "Initialising tasks...";

	// todo: put some stuff here
}


bool ServerApplication::help_cmd()
{
	std::cout << "Usage:" << std::endl
		<< "`.prove N`\tCreate a new process to prove"
		" N statements." << std::endl <<
		"`.help`,`.h`\tDisplay help message." << std::endl
		<< "`.exit`\tStop the server."
		<< std::endl;

	return true;
}


bool ServerApplication::prove_cmd(int n)
{
	if (n <= 0)
	{
		std::cout << "Number of statements needs to be > 0" <<
			std::endl;
		return false;
	}

	// TEMP
	std::cout << "Prove " << n << "!" << std::endl;

	return true;
}


bool ServerApplication::exit_cmd()
{
	ATP_ASSERT(!m_done);

	// indicate done:
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		m_done = true;
	}

	// join workers:
	for (size_t i = 0; i < m_workers.size(); ++i)
	{
		m_workers[i].join();
	}

	return true;
}


