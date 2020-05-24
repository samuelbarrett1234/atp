#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a process which generates random successors from
	theorems which are already known to be true (thus avoiding the
	task of looking for a proof - we get that for free).
*/


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
\brief Create a process which generates random successors from
	theorems already known to be true.

\details This is a cheap way of generating true theorems. However,
	since it is uninformed, we have no way of telling which generated
	theorems are better than others - so the generated theorems are
	guaranteed to be true but not necessarily useful.

\param N The number of proven theorems to load from the database,
	each of which we will generate one successor for.

\param depth The depth of the successor (the higher this is, the
	further away the generated results will be. 1 means it will only
	look for immediate successors).

\pre N > 0 && depth > 0
*/
ATP_CORE_API ProcessPtr create_uninformed_wanderer_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db, size_t N, size_t depth);


}  // namespace core
}  // namespace atp


