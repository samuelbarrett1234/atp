#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process for training the HMM conjecturer model on
	existing proven theorems.

*/


#include <vector>
#include <sstream>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "../Models/HMMConjectureModelBuilder.h"


namespace atp
{
namespace core
{


/**
\brief Create a process to train a particular HMM model
*/
ATP_CORE_API ProcessPtr create_hmm_conjecture_train_process(
	db::DatabasePtr p_db,
	logic::LanguagePtr p_lang, size_t ctx_id,
	logic::ModelContextPtr p_ctx, size_t num_epochs,
	size_t max_dataset_size,
	boost::optional<size_t> model_id = boost::none);


}  // namespace core
}  // namespace atp


