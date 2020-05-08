#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which loads the training dataset for the
	HMM conjecturer model from the database.

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
\brief Creates a process which loads a set of training data for the
	given model.

\param num_epochs The number of passes to perform over the dataset
	during training.

\param max_dataset_size The upper limit on the number of statements
	to load from the database to train on.

\pre All the data in `model_data` is valid.

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_hmm_training_data_load_proc(
	size_t num_epochs, size_t max_dataset_size,
	proc_data::HMMConjModelEssentials& model_data,
	proc_data::HMMConjecturerTrainingEssentials& train_data);


}  // namespace core
}  // namespace atp


