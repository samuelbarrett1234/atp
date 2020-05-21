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
	m_done(false), m_scheduler_on(true),
	m_proc_queue(std::make_unique<atp::core::ProcessQueue>())
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
	cmd_set.help_cmd = boost::bind(
		&ServerApplication::help_cmd, this);
	cmd_set.exit_cmd = boost::bind(
		&ServerApplication::exit_cmd, this);
	cmd_set.set_threads_cmd = boost::bind(
		&ServerApplication::set_threads_cmd, this, _1);
	cmd_set.set_scheduler_cmd = boost::bind(
		&ServerApplication::set_scheduler, this, _1);

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

		// run the scheduler if it is enabled

		const auto sizes = m_proc_queue->size();
		const size_t num_procs = sizes.get<0>() + sizes.get<1>() +
			sizes.get<2>();  // get total number of procs
		std::list<atp::core::ProcessPtr> scheduled_procs;

		if (m_scheduler_on && m_scheduler->update(scheduled_procs,
			num_procs))
		{
			// should return true iff procs were added
			ATP_ASSERT(scheduled_procs.size() > 0);

			std::cout << "Scheduler is adding " <<
				scheduled_procs.size() << " new process(es)."
				<< std::endl;

			// todo: maybe call `update` slightly less frequently

			for (auto&& p : scheduled_procs)
				m_proc_queue->push(std::move(p));
			scheduled_procs.clear();
		}
	}

	// workers are automatically joined by the exit command
}


void ServerApplication::worker_thread_func()
{
	// we will run processes N times every time we pop them from the
	// queue
#ifdef _DEBUG
	static const size_t N = 1;
#else
	static const size_t N = 10;
#endif

	using namespace std::chrono_literals;

	while (!is_done())
	{
		if (m_proc_queue->done())
		{
			// sleep to prevent spinning when we have no work to do
			std::this_thread::sleep_for(100ms);
		}
		else
		{
			// may be null!!
			auto p_proc = m_proc_queue->try_pop();

			// try again if we didn't get one:
			if (p_proc == nullptr)
				continue;

			ATP_CORE_ASSERT(!p_proc->done());
			for (size_t i = 0; i < N && !p_proc->done(); ++i)
			{
				p_proc->run_step();
			}

			// always put the process back onto the queue, regardless
			// of its current state
			m_proc_queue->push(std::move(p_proc));
		}
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

	if (m_scheduler_on)
	{
		std::list<atp::core::ProcessPtr> scheduled_procs;
		m_scheduler->update(scheduled_procs, 0);

		for (auto&& p : scheduled_procs)
			m_proc_queue->push(std::move(p));
	}
}


bool ServerApplication::help_cmd()
{
	ATP_LOG(trace) << "Printing help...";

	std::cout << "Usage:" << std::endl
		<< "`.help`,`.h`\t\tDisplay help message." << std::endl
		<< "`.ls`\t\t\tList information about processes which are "
		"currently running." << std::endl
		<< "`.exit`\t\t\tShut down the server." << std::endl
		<< "`.set_threads N`\tSet the number of threads to N."
		<< std::endl
		<< "`.set_scheduler on/off`\tEnable or disable automatic "
		"scheduling of new tasks as old ones get completed."
		<< std::endl;

	return true;
}


bool ServerApplication::ls_cmd()
{
	ATP_LOG(trace) << "Listing information about active server "
		"processes...";

	const auto q_sizes = m_proc_queue->size();

	std::cout << "There are currently "
		<< q_sizes.get<0>() << " processes in the queue, "
		"an extra " << q_sizes.get<1>() << " demoted to the "
		"waiting queue, and " << q_sizes.get<2>() <<
		" processes being actively worked on." << std::endl;

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

	std::cout << "Exiting, please wait..." << std::endl;

	// join workers:
	for (size_t i = 0; i < m_workers.size(); ++i)
	{
		m_workers[i].join();
	}

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

		std::cout << "Setting threads, please wait..." << std::endl;

		// join all workers:
		for (size_t i = 0; i < m_workers.size(); ++i)
		{
			m_workers[i].join();
		}

		// do NOT reset the process queue, or we will abort active
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


bool ServerApplication::set_scheduler(bool scheduler_on)
{
	const std::string cur = scheduler_on ? "on" : "off";
	const std::string last = m_scheduler_on ? "on" : "off";

	m_scheduler_on = scheduler_on;

	std::cout << "Switched scheduler " << cur << " (from "
		<< last << ")" << std::endl;

	return true;
}


