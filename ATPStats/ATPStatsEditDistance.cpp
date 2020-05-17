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
	float match_benefit, float unmatch_cost)
{
	switch (lang_type)
	{
	case logic::LangType::EQUATIONAL_LOGIC:
		return std::make_shared<EquationalEditDistanceTracker>(
			match_benefit, unmatch_cost);

	default:
		ATP_STATS_PRECOND(false && "Bad logic type!");
		return nullptr;
	}
}


}  // namespace stats
}  // namespace atp


