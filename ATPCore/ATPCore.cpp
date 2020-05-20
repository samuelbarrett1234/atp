/**
\file

\author Samuel Barrett

*/


#include "ATPCore.h"
#include "Scheduling/SimpleScheduler.h"


namespace atp
{
namespace core
{


SchedulerPtr create_scheduler(
	db::DatabasePtr p_db)
{
	ATP_CORE_PRECOND(p_db != nullptr);

	return std::make_unique<SimpleScheduler>(std::move(p_db));
}


}  // namespace core
}  // namespace atp


