#pragma once


/*
\file

\author Samuel Barrett

\brief Contains a simple object for juggling processes concurrently.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "ProcessQueue.h"


namespace atp
{
namespace core
{


/**
\brief This is a thread-safe class for juggling concurrent processes.

\details This class is used by "committing" threads to execute
	all the processes in it until they are all finished.
*/
class ATP_CORE_API ProcessManager
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
	void commit_thread();

private:
	ProcessQueue m_queue;
};


}  // namespace core
}  // namespace atp


