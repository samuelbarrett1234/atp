#pragma once


/**
\file

\author Samuel Barrett

*/


#include "IProcess.h"
#include "CommonProcessData.h"


namespace atp
{
namespace core
{


/**
\brief Create a process which selects a context from the database (at
	random, basically) for which it would be good to prove in.

\details This process is free to use statistical methods to pick the
	context, but randomness can be involved, so that this process
	executes the command "find me a good context to prove stuff in"
	satisfactorily.

\returns A new process
*/
ATP_CORE_API ProcessPtr create_select_ctx_process(
	proc_data::DatabaseEssentials& setup_data,
	proc_data::LogicEssentials& proof_data);


}  // namespace core
}  // namespace atp


