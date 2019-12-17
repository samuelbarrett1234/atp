#pragma once


// Author: Samuel Barrett


#include "ProcessStatus.h"
#include <string>
#include <memory>


namespace atpsearch
{


/// <summary>
/// A process represents a large computation which is to be
/// run alongside many other such computations (processes)
/// all of which compete for access to a small set of
/// resources. These are managed by a process manager which
/// only allows a process to tick when it has access to the
/// resources it has requested.
/// In order to work well, processes must be careful when
/// trying to request access to many resources, and must
/// not spend too long in the tick() function.
/// </summary>
class IProcess
{
public:
	virtual ~IProcess() = default;

	/// <summary>
	/// This function is called on the process regularly, and is
	/// where the process should do a bit of updating. It is
	/// important that the process doesn't block for too long, to
	/// be fair to other processes that also need running.
	/// </summary>
	/// <returns>
	/// For expensive operations like I/O, you can specify this in
	/// the returned process status object, so that it can be performed
	/// in parallel without blocking other processes.
	/// </returns>
	virtual ProcessStatus tick() = 0;

	/// <summary>
	/// This is called when the process manager decides to abort
	/// the process. This can be done when an I/O operation fails
	/// or when the process manager is stopped.
	/// </summary>
	virtual void abort() = 0;

	/// <summary>
	/// Get the process "name"/"type" for human processing.
	/// </summary>
	/// <returns>A human-readable process name</returns>
	virtual std::string get_name() const = 0;

	/// <summary>
	/// Get a detailed description for human processing.
	/// </summary>
	/// <returns>A human-readable collection of details about the process.</returns>
	virtual std::string get_details() const = 0;
};


typedef std::unique_ptr<IProcess> ProcessPtr;


} // namespace atpsearch


