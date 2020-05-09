#pragma once


/**
\file

\author Samuel Barrett

\brief A process for proving a set of statements.

*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPSearch.h>
#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


/**
\brief Create a proof process to try and prove `p_target_stmts`

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_proof_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db,
	search::SearchSettings& search_settings,
	logic::StatementArrayPtr p_target_stmts);


/**
\brief Create a proof process to try and prove `num_targets`
	arbitrary unproven theorems.

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_proof_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db,
	search::SearchSettings& search_settings,
	size_t num_targets);


}  // namespace core
}  // namespace atp


