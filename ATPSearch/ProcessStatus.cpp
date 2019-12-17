// Author: Samuel Barrett


#include "ProcessStatus.h"


namespace atpsearch
{


ProcessStatus finish_after(ProcessStatus stat)
{
    stat.status = ProcessStatus::Status::FINISH;
    return stat;
}


} // namespace atpsearch


