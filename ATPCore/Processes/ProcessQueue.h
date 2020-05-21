#pragma once


/*
\file

\author Samuel Barrett

\brief Contains an object for managing a queue of processes which may
	be in running/waiting/done states.

*/


#include <set>
#include <mutex>
#include <queue>
#include <boost/tuple/tuple.hpp>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


/**
\brief A thread-safe process queue, which on the surface acts
	like a queue, but has functionality for not updating waiting
	processes as often.

\details It returns waiting processes less often, and also keeps
	track of processes which are currently being worked on by
	other threads, even if they are not in the queue!
*/
class ATP_CORE_API ProcessQueue
{
public:
	ProcessQueue();

	/**
	\brief Either add a new process externally, or called by a
		worker thread which has finished updating its process.

	\post If `p_proc` was previously returned from a `try_pop`
		call, it will be removed from the set of currently-
		executing processes.
	*/
	void push(ProcessPtr p_proc);

	/**
	\brief Try to get a process which isn't being handled.

	\returns Nullptr if couldn't get a process, otherwise returns
		a process to handle.
	*/
	ProcessPtr try_pop();

	/**
	\brief Returns true if the stacks are empty AND no threads
		are currently working on processes that will subsequently
		be added to the queue.

	\warning **Do not** use this function to test whether
		`try_pop` will return null or not before actually calling
		it.
	*/
	bool done() const;

	/**
	\brief Returns the number of processes in the queue, and the
		ones which have been temporarily popped from the queue, and
		the number currently being executed, in that order.
	*/
	boost::tuple<size_t, size_t, size_t> size() const;

private:
	// invariant: i < N, and whenever i == 0, we prioritise the
	// waiting queue over the running queue.
	size_t i;
	const size_t N;
	mutable std::mutex m_mutex;
	std::queue<ProcessPtr> m_running, m_waiting;
	std::set<const IProcess*> m_executing;
};


}  // namespace core
}  // namespace atp


