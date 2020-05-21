#pragma once


/**

\file

\author Samuel Barrett

\brief Contains a command line application which is intended to be
	left running continuously, and automatically schedules its own
	processes.

*/


#include "ATP.h"
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include <ATPLogic.h>
#include <ATPSearch.h>
#include <ATPDatabase.h>
#include <ATPCore.h>


/**
\brief A command line application which is intended to be left
	running continuously, supporting many threads running processes
	asynchronously, and schedules its own processes.

\details The scheduling algorithm is factored out into a seperate
	list of classes. Furthermore, as this application runs
	continuously, instead of having an array of command line params,
	we use the main thread for console input only, and run the
	processes on all the worker threads.
*/
class ServerApplication
{
public:
	/**
	\param num_threads The number of WORKER threads, excluding the
		main thread.

	\pre num_threads > 0
	*/
	ServerApplication(size_t num_threads);

	/**
	\brief Loads the database file (this has to be done first)

	\returns True iff success.
	*/
	bool set_db(const std::string& path);

	/**
	\brief Let the processes run! This blocks until the user commands
		us to exit.

	\pre `run` can only be called once in this object's lifetime
	*/
	void run();

private:
	/**
	\brief This is the function that the worker threads spend their
		time in.
	*/
	void worker_thread_func();

	/**
	\brief Thread-safe getter function for m_done
	*/
	bool is_done() const;

	/**
	\brief Called once at the start of `run`; fills the process
		manager with some tasks to get going.
	*/
	void initialise_tasks();

private:  // COMMANDS

	bool help_cmd();
	bool ls_cmd();
	bool exit_cmd();
	bool set_threads_cmd(int n);
	bool set_scheduler(bool on);

private:
	bool m_done;  // indicates exit for worker threads
	mutable boost::shared_mutex m_mutex;  // for worker threads
	std::vector<std::thread> m_workers;
	std::unique_ptr<atp::core::ProcessQueue> m_proc_queue;
	atp::db::DatabasePtr m_db;
	atp::core::SchedulerPtr m_scheduler;
	bool m_scheduler_on;  // false means don't schedule things
	size_t m_num_completed;  // track number of finished procs
};


