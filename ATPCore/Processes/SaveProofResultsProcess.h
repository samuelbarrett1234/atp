#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which saves any proven theorems in the
	results of a solver's proof run.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"


namespace atp
{
namespace core
{


/**
\brief This function creates a process which saves proven theorems to
	the database.

\pre All the data in `proof_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_save_results_process(
	proc_data::ProofEssentials& proof_data);


}  // namespace core
}  // namespace atp


