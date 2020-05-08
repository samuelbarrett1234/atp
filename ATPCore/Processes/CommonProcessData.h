#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a collection of structs which contain data that is
	used in a lot of different processes.

*/


#include <ATPLogic.h>
#include <ATPDatabase.h>
#include <ATPSearch.h>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
{
namespace proc_data
{


struct ATP_CORE_API DatabaseEssentials
{
	db::DatabasePtr db;
};


struct ATP_CORE_API LogicEssentials :
	public DatabaseEssentials
{
	logic::LanguagePtr lang;
	logic::ModelContextPtr ctx;
	size_t ctx_id;
};


struct ATP_CORE_API ProofSetupEssentials :
	public LogicEssentials
{
	search::SearchSettings settings;
	size_t ss_id, num_helper_thms;
	logic::StatementArrayPtr target_thms;
};


struct ATP_CORE_API ProofEssentials :
	public LogicEssentials
{
	logic::KnowledgeKernelPtr ker;
	search::SolverPtr solver;
	size_t ss_id;
	logic::StatementArrayPtr helper_thms;
};


}  // namespace proc_data
}  // namespace core
}  // namespace atp


