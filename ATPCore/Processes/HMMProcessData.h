#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a collection of structs which contain data that is
	used in a lot of different processes.

*/


#include <boost/optional.hpp>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include <ATPSearch.h>
#include "../ATPCoreAPI.h"
#include "CommonProcessData.h"
#include "../Models/HMMConjectureModelBuilder.h"


namespace atp
{
namespace core
{
namespace proc_data
{


struct ATP_CORE_API HMMConjBuildingEssentials :
	public LogicEssentials
{
	boost::optional<size_t> model_id;
};


struct ATP_CORE_API HMMConjModelEssentials :
	public LogicEssentials
{
	std::unique_ptr<HMMConjectureModel> model;
	size_t model_id;
};


struct ATP_CORE_API HMMConjecturingEssentials :
	public HMMConjModelEssentials
{
	size_t num_to_generate;
};


struct ATP_CORE_API HMMConjecturerTrainingEssentials :
	public HMMConjModelEssentials
{
	size_t num_epochs;
	logic::StatementArrayPtr dataset;
};


}  // namespace proc_data
}  // namespace core
}  // namespace atp


