#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which saves any conjectures produced by
	the HMM conjecturer model.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"
#include "HMMProcessData.h"


namespace atp
{
namespace core
{


/**
\brief This function creates a process which saves proven theorems to
	the database.

\pre All the data in `gen_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_save_results_process(
	proc_data::HMMConjGenerationEssentials& gen_data);


}  // namespace core
}  // namespace atp


