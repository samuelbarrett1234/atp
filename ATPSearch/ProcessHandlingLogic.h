#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "ProcessScheduler.h"
#include "ResourceOperationScheduler.h"


namespace atpsearch
{


/// <summary>
/// Handles the internal logic of Process Manager.
/// This logic has been factored out into this class
/// for simplicity and to allow the logic to be
/// tested.
/// Every function in this class is thread-safe.
/// </summary>
class ATP_API ProcessHandlingLogic
{
public:
	ProcessHandlingLogic(ProcessSchedulerType proc_scheduler_type,
		ResourceOperationSchedulerType res_scheduler_type);

	/// <summary>
	/// Call this when a brand new process is posted
	/// to the process manager.
	/// Precondition: worker_id has not already been added.
	/// </summary>
	/// <param name="worker_id">Some ID of the process.</param>
	virtual void add_new_process(size_t worker_id);

	/// <summary>
	/// Try to get the next worker ID to handle. If there
	/// is one available and p_worker_id is non-null, then
	/// a worker will be written out to this parameter.
	/// This function returns true iff there are any workers
	/// ready to be executed.
	/// </summary>
	/// <param name="p_worker_id">The ID of the next worker.</param>
	/// <returns>True if there were any workers remaining, false if not.</returns>
	virtual bool next_worker_id_if_exists(size_t* p_worker_id);

	/// <summary>
	/// Add some resource operations given by a worker.
	/// </summary>
	/// <param name="p_begin">Beginning of an array of resource operations.</param>
	/// <param name="p_end">End of an array of resource operations (one past the end).</param>
	/// <param name="worker_id">The worker that produced these operations.</param>
	virtual void add_resource_operations(ResourceOperation* p_begin,
		ResourceOperation* p_end, size_t worker_id);

	/// <summary>
	/// Try to get the next resource operation to execute. If
	/// p_res_op is non-null, then a res-op is written to that
	/// pointer if one is available. True is returned iff a
	/// res-op is available.
	/// </summary>
	/// <param name="p_res_op">Where to write the next resource operation.</param>
	/// <returns>True iff there was any res-op available.</returns>
	virtual bool next_resop_if_exists(ResourceOperation* p_res_op);



};


} // namespace atpsearch


