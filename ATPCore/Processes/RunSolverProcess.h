#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which takes setup information about the
	proof setup, and creates all the information needed to carry
	out the proof.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"


namespace atp
{
namespace core
{


/**
\brief This function creates a process which just runs the solver to
	completion.

\pre All the data in `proof_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_run_solver_process(
	proc_data::ProofEssentials& proof_data_before,
	proc_data::ProofEssentials& proof_data_after);


}  // namespace core
}  // namespace atp


