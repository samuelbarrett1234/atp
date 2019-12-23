#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "ResourceOperation.h"
#include <memory>


namespace atpsearch
{


/// <summary>
/// This class is a helper class whose lifetime represents
/// the duration of a resource operation's work. It represents
/// a way of getting the ID of a resource operation, and also
/// when this object destructs, it notifies the scheduler that
/// the resource operation's work has finished.
/// </summary>
class ATP_API ResourceOperationWork
{
public:
	ResourceOperationWork(size_t resop_id, IResourceOperationScheduler& scheduler);
	~ResourceOperationWork();

	size_t operator* () const;
};


/// <summary>
/// This class has the responsibility of distributing resource operations.
/// It is basically a central, thread-safe queue for resource operations.
/// It also allows IO operations to depend on other IO operations, and
/// only allows an IO operation to begin when the operation it depends on
/// finishes.
/// Does not manage resource clashes (this should be managed higher-up).
/// </summary>
class ATP_API IResourceOperationScheduler
{
public:
	virtual ~IResourceOperationScheduler() = default;

	/// <summary>
	/// Add a resource operation to the scheduler.
	/// Precondition: the res_op_id must not have been added
	/// before. Must be not equal to -1 as this is reserved as
	/// a "not-an-ID".
	/// </summary>
	/// <param name="res_op_id">The new ID of the resource operation (must
	/// be unique w.r.t. for this res-op.)</param>
	/// <param name="depends_on_id">
	/// An optional ID of another resource operation which must be
	/// completed before this.
	/// If such an ID does not exist it will be assumed that the
	/// operation has been completed and the dependency will be ignored.
	/// </param>
	virtual void add(size_t res_op_id, size_t depends_on_id = (size_t)(-1)) = 0;

	/// <summary>
	/// Determine if there are any resource operations ready to be
	/// executed.
	/// </summary>
	/// <returns>True if there is one ready, false if not.</returns>
	virtual bool ready() const = 0;

	/// <summary>
	/// Get the next available resource operation.
	/// This function will never return the same ID twice
	/// (because we don't want several threads executing
	/// the same operation.)
	/// Precondition: ready() returns true.
	/// </summary>
	/// <returns>
	/// A "resource operation work" object which allows you to
	/// get the ID of the resource operation (by using *) and
	/// also the lifetime of this work object represents the
	/// lifetime of the resource operation's work (so it calls
	/// on_finished() in the destructor.)
	/// </returns>
	virtual ResourceOperationWork next() = 0;

protected:
	friend ResourceOperationWork;

	/// <summary>
	/// Call this when an resource operation has finished,
	/// to allow any res-ops which depend on it to be ran.
	/// </summary>
	/// <param name="res_op_id">
	/// The ID of the resource operation which has just finished.
	/// </param>
	virtual void on_finished(size_t res_op_id) = 0;
};


typedef std::unique_ptr<IResourceOperationScheduler> ResourceOperationSchedulerPtr;


/// <summary>
/// Different types of scheduler algorithm for processes
/// </summary>
enum class ResourceOperationSchedulerType
{
	// When a thread runs out of processes it will steal
	// one from another thread at random (hence if the
	// queue for ANY thread is nonempty, ANY OTHER thread
	// will always be able to find work.)
	WORK_STEALING
};


ATP_API ResourceOperationSchedulerPtr create_resop_scheduler(ResourceOperationSchedulerType type);


} // namespace atpsearch


