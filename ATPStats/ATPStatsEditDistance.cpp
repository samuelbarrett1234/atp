/**
\file

\author Samuel Barrett

*/


#include "ATPStatsEditDistance.h"
#include "Internal/EquationalEditDistance.h"


namespace atp
{
namespace stats
{


EditDistancePtr create_edit_dist(
	logic::LangType lang_type,
	EditDistSubCosts sub_costs)
{
	switch (lang_type)
	{
	case logic::LangType::EQUATIONAL_LOGIC:
		return std::make_shared<EquationalEditDistanceTracker>(
			std::move(sub_costs));

	default:
		ATP_STATS_PRECOND(false && "Bad logic type!");
		return nullptr;
	}
}


}  // namespace stats
}  // namespace atp


