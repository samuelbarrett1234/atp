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


ATP_CORE_API ProcessPtr create_proof_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, size_t ss_id,
	logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db,
	search::SearchSettings& search_settings,
	logic::StatementArrayPtr p_target_stmts);


}  // namespace core
}  // namespace atp


