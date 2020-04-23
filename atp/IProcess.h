#pragma once


/**
\file

\author Samuel Barrett

\brief File containing interfaces for process management

*/


#include <memory>
#include <ostream>


/*
\enum ProcessState

\brief The different states of a process
*/
enum class ProcessState
{
	AWAITING_LOCK,
	RUNNING,
	DONE
};


/**
\brief Represents a computation to be performed over time.

\details Processes may require locks, in which case they may decide
	to begin in an AWAITING_LOCK state. Some processes have an end
	point, in which case they will eventually transition to DONE,
	however other processes may want to return to AWAITING_LOCK if
	they need different resources.

\warning Processes should never hold more than one lock.
*/
class IProcess
{
public:
	virtual ~IProcess() = default;

	/**
	\brief Get the state of the process
	*/
	virtual ProcessState state() const = 0;

	/**
	\brief Perform a step of the computation.

	\pre state() == ProcessState::RUNNING

	\post The process may wish to revert back to the AWAITING_LOCK
		state, in which case it will be paused and put to the back
		of the awaiting-processes queue.

	\post If the process finishes, the process should transition to
		the ProcessState::DONE state.
	*/
	virtual void run_step() = 0;

	/**
	\brief Make a single attempt at trying to acquire the locks that
		this process needs.

	\pre state() == ProcessState::AWAITING_LOCK

	\post If the lock was acquired, the process should transition to
		the ProcessState::RUNNING state.
	*/
	virtual void try_acquire_lock() = 0;

	/**
	\brief Give the process a chance to write any information it has
		to the given logs.

	\details The process can assume that it is the sole owner of this
		output stream. The processes will have this called
		periodically, but in between calls the processes should
		keep track of what information they would like to output. Of
		course, processes are not obliged to do anything here, but it
		helps contribute to user output.
	*/
	virtual void dump_log(std::ostream& out) = 0;
};


typedef std::shared_ptr<IProcess> ProcessPtr;


