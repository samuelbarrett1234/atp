#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a process which saves all HMM model parameters to the
	database.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"
#include "HMMProcessData.h"


namespace atp
{
namespace core
{


ATP_CORE_API ProcessPtr create_save_hmm_model_params_proc(
	proc_data::HMMConjecturerTrainingEssentials& train_data);


}  // namespace core
}  // namespace atp


