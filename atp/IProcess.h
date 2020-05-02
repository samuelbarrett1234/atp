#pragma once


/**
\file

\author Samuel Barrett

\brief File containing interfaces for process management

*/


#include <memory>
#include <ostream>


/**
\brief Represents a computation to be performed over time.

\details Processes may require locks, in which case they may decide
	to begin in an WAITING state. Some processes have an end
	point, in which case they will eventually transition to DONE,
	however other processes may want to return to WAITING if
	they need different resources.

\warning Processes should never hold more than one lock.
*/
class IProcess
{
public:
	virtual ~IProcess() = default;

	/**
	\brief Returns true iff the process has finished.
	*/
	virtual bool done() const = 0;

	/**
	\brief Returns true iff the process is waiting on some resources.

	\details Processes are not obliged to ever return true here; this
		is purely just a hint for the process manager.

	\post If waiting() is true, then done() should be false.
	*/
	virtual bool waiting() const = 0;

	/**
	\brief Perform a step of the computation.

	\pre !done()

	\post This may cause the return values of done() and waiting() to
		change.
	*/
	virtual void run_step() = 0;

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


