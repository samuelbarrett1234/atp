#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a process which loads N arbitrary unproven theorems
	from the database.
*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"


namespace atp
{
namespace core
{


/**
\brief Create a process which loads a number of theorems into
	`target_thms`.

\pre input.target_thms == nullptr

\post When this process finishes, output.target_thms will be
	initialised with at most `num_to_load` theorems.

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_unproven_thm_select_proc(
	size_t num_to_load,
	proc_data::ProofSetupEssentials& input,
	proc_data::ProofSetupEssentials& output);


}  // namespace core
}  // namespace atp


