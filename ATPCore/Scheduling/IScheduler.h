#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the interface for objects responsible for scheduling
	and balancing different ATP tasks.

*/


#include <list>
#include <memory>
#include "../ATPCoreAPI.h"
#include "../Processes/IProcess.h"


namespace atp
{
namespace core
{


/**
\brief A scheduler is responsible for periodically adding tasks to
	the process queue, to ensure that all the threads are
	appropriately busy.

\details Since computing what to schedule might be complex and
	involve calls to the database, schedulers delegate these tasks
	to processes of their own - so the scheduler is effectively an
	object for (i) creating processes which calculate what to
	schedule, and (ii) determining when it is necessary to do so.
*/
class ATP_CORE_API IScheduler
{
public:
	virtual ~IScheduler() = default;

	/**
	\brief Add scheduling processes if necessary.

	\details You should call this function at fairly regular
		intervals, although not doing so is not that bad; it just
		means that as soon as you run out of work, this function will
		make sure you have work to do.

	\param out_procs The place to put any new processes if the
		scheduler wishes.

	\param num_procs A count of the total number of processes
		currently present, so the scheduler can get an idea of how
		many more are needed.

	\returns True if processes were added, false if nothing was added

	\note The scheduling process adds to the process queue by the
		time it finishes. Feel free to get rid of the scheduler once
		this has been called, as the scheduling process is given all
		the information it needs when it is constructed. This has the
		downside of not updating information (like the number of
		active threads) if it changes, but the scheduling process
		shouldn't be alive for long in real-time.
	*/
	virtual bool update(std::list<ProcessPtr>& out_procs,
		size_t num_procs) = 0;

	/**
	\brief Indicate the number of threads that are currently being
		used to run processes.

	\details This is used for calculation purposes only, so failing
		to keep this value up-to-date is not by any means fatal, but
		will mean the threads get more or less work than they should.
		The default (before you call this function) is 1.

	\pre nt > 0, because why would you ever use 0 worker threads yet
		still want to get work done?
	*/
	virtual void set_num_threads(size_t nt) = 0;
};


typedef std::unique_ptr<IScheduler> SchedulerPtr;


}  // namespace core
}  // namespace atp


