#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which loads the HMM conjecturer model
	from the database.

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
\brief This function creates a process which loads a HMM conjecturer
	model from the database.

\pre All the data in `building_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_hmm_model_loader_process(
	proc_data::HMMConjBuildingEssentials& building_data,
	proc_data::HMMConjModelEssentials& model_data);


}  // namespace core
}  // namespace atp


