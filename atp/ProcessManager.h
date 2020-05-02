#pragma once


/*
\file

\author Samuel Barrett

\brief Contains an object for juggling processes concurrently.

*/


#include <mutex>
#include <queue>
#include <ATPDatabase.h>
#include "IProcess.h"


/**
\brief This is a thread-safe class for juggling concurrent processes.

\details This class is used by "committing" threads to execute
	all the processes in it until they are all finished.
*/
class ProcessManager
{
public:
	/**
	\brief Add the given process to the mix.
	*/
	void add(ProcessPtr p_proc);

	/**
	\brief Make the current thread work for the process manager.

	\details This will block until the process manager is empty.
	*/
	void commit_thread(std::ostream& output);

private:
	bool done() const;
	ProcessPtr pop_running();
	ProcessPtr pop_waiting();
	void push_running(ProcessPtr p_proc);
	void push_waiting(ProcessPtr p_proc);

private:
	mutable std::mutex m_mutex;
	std::queue<ProcessPtr> m_running, m_waiting;
};


