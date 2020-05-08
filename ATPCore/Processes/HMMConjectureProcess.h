#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an implementation of a process which runs an
	automated conjecturing procedure.
*/


#include <sstream>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "../Models/HMMConjectureModel.h"
#include "../Models/HMMConjectureModelBuilder.h"


namespace atp
{
namespace core
{


/**
\brief Create a process which generates a number of conjectures using
	the HMM conjecture model.
*/
ATP_CORE_API ProcessPtr create_hmm_conjecture_process(
	db::DatabasePtr p_db,
	logic::LanguagePtr p_lang, size_t ctx_id,
	logic::ModelContextPtr p_ctx, size_t num_to_generate,
	boost::optional<size_t> model_id = boost::none);


}  // namespace core
}  // namespace atp


