#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "ResourceOperation.h"
#include <vector>
#include <initializer_list>


namespace atpsearch
{


//Export arrays of resource operations:
template class ATP_API std::vector<ResourceOperation>;


/// <summary>
/// This is returned by a process from its tick() function,
/// and it indicates the current state of the process (i.e.
/// either still running or finished) and also contains
/// information for any asynchronous operations that the
/// process wants to execute for its next iteration.
/// </summary>
struct ATP_API ProcessStatus
{
    bool bFinished = false;
    std::vector<ResourceOperation> ops;
};


namespace status // this namespace contains a set of convenience functions for constructing ProcessStatus objects.
{


ATP_API ProcessStatus not_finished();

ATP_API ProcessStatus res_op(ResourceOperation op);
ATP_API ProcessStatus res_ops(std::initializer_list<ResourceOperation> ops);
ATP_API ProcessStatus res_ops(std::vector<ResourceOperation> ops);

ATP_API ProcessStatus finished();

ATP_API ProcessStatus finish_after_res_op(ResourceOperation op);
ATP_API ProcessStatus finish_after_res_ops(std::initializer_list<ResourceOperation> ops);
ATP_API ProcessStatus finish_after_res_ops(std::vector<ResourceOperation> ops);


} // namespace status


} // namespace atpsearch


