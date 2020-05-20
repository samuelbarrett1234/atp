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
\brief Create a process which selects search settings from the
	database (at random, basically) for which it would be good to
	prove with.

\details This process is free to use statistical methods to pick the
	settings, but randomness can be involved, so that this process
	executes the command "find me good settings to prove stuff with"
	satisfactorily.

\post The process does not fill in the target theorems, but does fill
	in the proof setup data.

\returns A new process
*/
ATP_CORE_API ProcessPtr create_select_ss_process(
	proc_data::LogicEssentials& logic_data,
	proc_data::ProofSetupEssentials& setup_data);


}  // namespace core
}  // namespace atp


