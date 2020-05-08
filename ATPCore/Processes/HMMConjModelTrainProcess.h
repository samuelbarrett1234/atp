#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a process which trains a given HMM conjecture model
	with a given dataset.

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
\brief Create a process which trains a given model on a given dataset
*/
ATP_CORE_API ProcessPtr create_conj_model_train_process(
	proc_data::HMMConjecturerTrainingEssentials& train_in,
	proc_data::HMMConjecturerTrainingEssentials& train_out);


}  // namespace core
}  // namespace atp


