#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "ProcessScheduler.h"
#include "ResourceOperationScheduler.h"
#include <boost/optional.hpp>


namespace atpsearch
{


class ProcessHandlingLogic;


/// <summary>
/// This struct contains information about a
/// process, and also executes code in the
/// destructor.
/// </summary>
struct ATP_API ProcessHandlingMetaData
{
public:
	ProcessHandlingMetaData(ProcessHandlingLogic& handler,
		size_t process_id,
		bool b_finished);
	~ProcessHandlingMetaData();

	const size_t process_id;  // a unique ID for the process
	bool b_finished;  // if true, the process is ready to be destroyed

	// If non-empty when the destructor is called, these will be added
	// to the pool of resource operations
	ResourceOperations res_ops;

private:
	ProcessHandlingLogic& handler;
};


struct ATP_API ResOpMetaData
{
public:
	ResOpMetaData(ProcessHandlingLogic& handler,
		ResourceOperation res_op,
		size_t process_id,
		size_t res_op_idx);
	~ResOpMetaData();

	ResourceOperation res_op;  // the main operation

	bool b_failed;  // starts as false; set to true if op failed

	// the process which this operation corresponds to
	const size_t process_id;

	// the index of this res-op in the list of res-ops for the process
	const size_t res_op_idx;
};


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
	/// Precondition: process_id has not already been added.
	/// </summary>
	/// <param name="process_id">Some ID of the process.</param>
	void add_new_process(size_t process_id);

	/// <summary>
	/// Try to get the next process to update. If there
	/// is one available, the returned option will contain
	/// a struct of data about the process. This struct
	/// will execute important code on exit - try to make
	/// sure it goes out of scope when you have finished
	/// handling the object.
	/// POSTCONDITIONS: if the returned option contains a
	/// value, that struct will return a valid process_id,
	/// and b_finished will be false, and res_ops will be
	/// empty.
	/// NOTE: if a process is marked as finished, and then
	/// the destructor of the struct is called, then the
	/// process will never be returned from this function
	/// again.
	/// </summary>
	/// <returns>
	/// If there were no processes available to be handled,
	/// this returns none. Otherwise, it returns a struct
	/// of data about the next process. Modify this as you
	/// wish (by indicating that it has finished / adding
	/// res-ops, etc) and then when the destructor is called,
	/// all of this data will be cached.
	/// </returns>
	boost::optional<ProcessHandlingMetaData> try_begin_next_process();

	/// <summary>
	/// Try to get the next res-op to complete. If there
	/// is one available, the returned option will contain
	/// a struct of data about the res-op. This struct
	/// will execute important code on exit - try to make
	/// sure it goes out of scope when you have finished
	/// handling the operation.
	/// POSTCONDITIONS: if the returned option contains a
	/// value, that struct will return a valid process_id,
	/// and res_op_idx. b_failed will always initially be false.
	/// NOTE: it is always assumed that you complete a res-op
	/// successfully, unless b_failed is set to true, in
	/// which case all other res-ops for this process are
	/// cleared, and the process is aborted. Earlier res-ops
	/// are undone, as well. This is all executed in the
	/// destructor.
	/// </summary>
	/// <returns>
	/// If there were no res-ops available to be handled,
	/// this returns none. Otherwise, it returns a struct
	/// of data about the next res-op. Modify this as you
	/// wish (by indicating that it has failed if it did)
	/// and then when the destructor is called, all of
	/// this data will be handled.
	/// </returns>
	boost::optional<ResOpMetaData> try_begin_next_res_op();


private:
	friend ProcessHandlingMetaData;
	friend ResOpMetaData;


	/// <summary>
	/// Once you have finished handling a process, call this.
	/// This should only need to be called by the
	/// ProcessHandlingMetaData struct, which does so in the
	/// destructor.
	/// </summary>
	/// <param name="data">
	/// The data corresponding to the process that just finished.
	/// </param>
	void end_next_process(ProcessHandlingMetaData& data);


	/// <summary>
	/// Once you have finished handling a res-op, call this.
	/// This should only need to be called by the ResOpMetaData
	/// struct, which does so in the destructor.
	/// </summary>
	/// <param name="data">
	/// The data corresponding to the res-op that just finished.
	/// </param>
	void end_next_res_op(ProcessHandlingMetaData& data);
};


} // namespace atpsearch


