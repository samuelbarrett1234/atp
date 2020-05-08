#pragma once


/**
\file

\author Samuel Barrett

\brief This process generates conjectures using a given HMM model

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
\brief This function creates a process which generates conjectures
	given a HMM conjecture model.

\pre All the data in `model_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_hmm_conj_gen_process(
	size_t num_to_generate,
	proc_data::HMMConjModelEssentials& model_data,
	proc_data::HMMConjGenerationEssentials& gen_data);


}  // namespace core
}  // namespace atp


