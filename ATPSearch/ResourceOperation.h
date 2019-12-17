#pragma once


// Author: Samuel Barrett


#include "Utility.h"
#include "Resource.h"
#include <vector>
#include <initializer_list>
#include <streambuf>


namespace atpsearch
{


/// <summary>
/// A resource operation is returned from a process tick()
/// and represents an I/O operation that is done on one or more
/// resources. Processes can return lists of these.
/// </summary>
struct ATP_API ResourceOperation
{
    enum Type
    {
        READ,
        WRITE,
        SEEK,
        PIPE
    };

    union
    {
        struct
        {
            ResourceID res;
            void* pBuffer;
            size_t bufSize, readSize;
            size_t* sizeRead;
        } read;

        struct
        {
            ResourceID res;
            const void* pBuffer;
            size_t writeSize;
            size_t* sizeWritten;
        } write;

        struct
        {
            ResourceID res;
            std::streamoff seekPos;
            std::ios_base::seek_dir seekDir;
        } seek;

        struct
        {
            ResourceID source, target;
            size_t transferSize;
            size_t* amountTransferred;
        } pipe;
    };

    //This value is useful only in the context of a list of
    // resource operations. It represents the index of the
    // resource operation which must be completed before this
    // one.
    //To ensure this graph is acyclic, dependsOn must be strictly
    // less than the index of the current operation, or -1 if it
    // doesn't depend on any.
    size_t dependsOn = DOESNT_DEPEND;
    static const size_t DOESNT_DEPEND = static_cast<size_t>(-1);
};


namespace resop // this namespace contains a set of convenience functions for constructing ResourceOperation objects.
{


ATP_API ResourceOperation read(ResourceID id, void* pBuffer, size_t readSize,
    size_t* sizeRead, size_t dependsOn = ResourceOperation::DOESNT_DEPEND);

ATP_API ResourceOperation write(ResourceID id, const void* pBuffer, size_t writeSize,
    size_t* sizeWritten, size_t dependsOn = ResourceOperation::DOESNT_DEPEND);

ATP_API ResourceOperation seek(ResourceID id, std::ios_base::seek_dir seekDir, std::streamoff seekAmount,
    size_t dependsOn = ResourceOperation::DOESNT_DEPEND);

ATP_API ResourceOperation pipe(ResourceID source, ResourceID target, size_t transferSize,
    size_t* sizeTransferred, size_t dependsOn = ResourceOperation::DOESNT_DEPEND);


} // namespace resop


} // namespace atpsearch


