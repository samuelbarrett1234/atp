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
	m_done(false),
	m_proc_mgr(std::make_unique<atp::core::ProcessManager>())
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
	cmd_set.ls_cmd = boost::bind(
		&ServerApplication::ls_cmd, this);
	cmd_set.killall_cmd = boost::bind(
		&ServerApplication::killall_cmd, this);
	cmd_set.help_cmd = boost::bind(
		&ServerApplication::help_cmd, this);
	cmd_set.exit_cmd = boost::bind(
		&ServerApplication::exit_cmd, this);
	cmd_set.set_threads_cmd = boost::bind(
		&ServerApplication::set_threads_cmd, this, _1);

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

		// also check this (we could consider reducing how often this
		// is called, slightly)
		m_scheduler->update(*m_proc_mgr);
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
		m_proc_mgr->commit_thread();

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
	ATP_ASSERT(m_db != nullptr);
	ATP_LOG(trace) << "Initialising tasks...";

	m_scheduler = atp::core::create_scheduler(m_db);

	m_scheduler->set_num_threads(m_workers.size());

	m_scheduler->update(*m_proc_mgr);
}


bool ServerApplication::help_cmd()
{
	ATP_LOG(trace) << "Printing help...";

	std::cout << "Usage:" << std::endl
		<< "`.help`,`.h`\tDisplay help message." << std::endl
		<< "`.ls`\tList information about processes which are "
		"currently running." << std::endl <<
		"`.killall`\tStop all processes, but don't shut down "
		"the server." << std::endl
		<< "`.exit`\t\tShut down the server." << std::endl
		<< "`.set_threads N`\t\tSet the number of threads to N."
		<< std::endl;

	return true;
}


bool ServerApplication::ls_cmd()
{
	ATP_LOG(trace) << "Listing information about active server "
		"processes...";

	std::cout << "There are currently "
		<< m_proc_mgr->num_procs_running()
		<< " processes running." << std::endl;

	return true;
}


bool ServerApplication::exit_cmd()
{
	ATP_ASSERT(!m_done);
	ATP_LOG(trace) << "Shutting down server...";

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


bool ServerApplication::killall_cmd()
{
	ATP_ASSERT(!m_done);
	ATP_LOG(trace) << "Killing all processes...";

	// pretend we're done:
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		m_done = true;
	}

	// join workers:
	for (size_t i = 0; i < m_workers.size(); ++i)
	{
		m_workers[i].join();
	}

	// reset proc mgr:
	m_proc_mgr.reset();

	// we're not actually done (the lock probably isn't necessary
	// here, but might as well code defensively!)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		m_done = false;
	}

	// recreate this
	m_proc_mgr = std::make_unique<atp::core::ProcessManager>();

	// relaunch the workers:
	for (size_t i = 0; i < m_workers.size(); ++i)
	{
		m_workers[i] = std::thread([this]()
			{ worker_thread_func(); });
	}

	// reinitialise tasks:
	initialise_tasks();

	ATP_LOG(trace) << "All processes killed.";

	return true;
}


bool ServerApplication::set_threads_cmd(int n)
{
	if (n <= 0)
	{
		ATP_LOG(error) << "Cannot set the number of server threads "
			"to " << n;
		return false;
	}

	ATP_LOG(trace) << "Attempting server set_threads with " << n <<
		"threads...";

	const size_t m = (size_t)n;

	if (m < m_workers.size())
	{
		ATP_LOG(trace) << "Reducing the number of worker threads "
			"from " << m_workers.size() << " to " << m << "...";

		// pretend we're done:
		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);

			m_done = true;
		}

		// join all workers:
		for (size_t i = 0; i < m_workers.size(); ++i)
		{
			m_workers[i].join();
		}

		// do NOT reset the process manager, or we will abort active
		// processes

		// we're not actually done:
		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);

			m_done = false;
		}

		// recreate workers:
		m_workers.clear();
		m_workers.reserve(m);
		for (size_t i = 0; i < m; ++i)
		{
			m_workers.emplace_back([this]()
				{ worker_thread_func(); });
		}

		ATP_LOG(trace) << "Done changing server worker threads.";
	}
	else if (m > m_workers.size())
	{
		ATP_LOG(trace) << "Increasing the number of worker threads "
			"from " << m_workers.size() << " to " << m << "...";

		m_workers.reserve(m);
		for (size_t i = 0; i < m - m_workers.size(); ++i)
		{
			m_workers.emplace_back([this]()
				{ worker_thread_func(); });
		}

		ATP_LOG(trace) << "Done changing server worker threads.";
	}
	else
	{
		ATP_LOG(trace) << "No change in the number of worker threads"
			" (" << m << "), so no action needed.";
	}

	m_scheduler->set_num_threads(m);

	return true;
}


