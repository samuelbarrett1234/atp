#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a process which creates new HMM models

*/


#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


/**
\brief Create a new HMM conjecturer in the database

\param num_hidden_states The number of hidden states that the HMM
	model should have.

\param model_id The ID to assign the new model

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_hmm_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db, size_t num_hidden_states,
	size_t model_id);


}  // namespace core
}  // namespace atp


