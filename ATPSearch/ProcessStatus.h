#pragma once


// Author: Samuel Barrett


#include "Utility.h"


namespace atpsearch
{


/// <summary>
/// This is returned by a process from its tick() function,
/// and it indicates the current state of the process (i.e.
/// either still running or finished) and also contains
/// information for any asynchronous operations that the
/// process wants to execute for its next iteration.
/// </summary>
struct ProcessStatus
{
    enum class Status
    {
        CONTINUE,
        FINISH
    }
    status = Status::CONTINUE;

    enum class Action
    {
        NONE,
        LOCK_REQUEST,
        RW_OP
    }
    action = Action::NONE;

    struct
    {
        // TODO
    } lock_request;

    struct
    {
        // TODO
    } rw_op;
};


/// <summary>
/// This is a convenience function which takes a process
/// status (e.g. an async IO operation) and adds a flag
/// to the status to say that, once this operation has
/// finished, the process is finished. This will only happen
/// AFTER the action completes.
/// </summary>
/// <param name="stat">The status to add the flag to.</param>
/// <returns>The process status as given, but with the status set to FINISH.</returns>
ATP_API ProcessStatus finish_after(ProcessStatus stat);


} // namespace atpsearch


