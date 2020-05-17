#pragma once


/**
\file

\author Samuel Barrett

*/


#include <memory>
#include <vector>
#include <ATPLogic.h>
#include "ATPStatsAPI.h"


namespace atp
{
namespace stats
{


/**
\brief An object for computing edit distance for statements in a
	particular logic type, which is optimised.

\details This class is **thread-safe**.
*/
class ATP_STATS_API IEditDistance
{
public:
	virtual ~IEditDistance() = default;

	virtual float edit_distance(
		const logic::IStatement& stmt1,
		const logic::IStatement& stmt2) = 0;

	virtual std::vector<std::vector<float>> edit_distance(
		const logic::IStatementArray& stmtarr1,
		const logic::IStatementArray& stmtarr2) = 0;
};
typedef std::shared_ptr<IEditDistance> EditDistancePtr;


/**
\brief Create a new edit distance calculator object for the given
	logic type and substitution costs.

\param match_benefit A nonnegative number representing the
	DECREASE in edit distance PER DEPTH OF MATCHING

\param unmatch_cost A nonnegative number representing the
	increase in edit distance when a pair of symbols are not
	matched.
*/
ATP_STATS_API EditDistancePtr create_edit_dist(
	logic::LangType lang_type,
	float match_benefit, float unmatch_cost);


}  // namespace stats
}  // namespace atp


