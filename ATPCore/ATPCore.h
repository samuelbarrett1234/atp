#pragma once


/*

\file

\author Samuel Barrett

\brief Main header file for this library.

*/


#include <ATPDatabase.h>
#include "ATPCoreAPI.h"
#include "Processes/IProcess.h"
#include "Processes/ProcessManager.h"
#include "Processes/ProofProcess.h"
#include "Processes/HMMConjectureProcess.h"
#include "Processes/HMMConjectureTrainProcess.h"
#include "Processes/CreateHMMProcess.h"
#include "Scheduling/IScheduler.h"


/**
\namespace atp::core

\brief The namespace containing all processes and operations
	performed by ATP user applications
*/


namespace atp
{
namespace core
{


/**
\brief Allocate a new scheduler object.
*/
ATP_CORE_API SchedulerPtr create_scheduler(
	db::DatabasePtr p_db);


}  // namespace core
}  // namespace atp


