#pragma once


/**
\file

\author Samuel Barrett

\brief This file contains edit distance computations specialised to
	equational logic.
*/


#include <unordered_map>
#include <ATPLogic.h>
#include <Internal/Equational/Expression.h>
#include "../ATPStatsAPI.h"
#include "../ATPStatsEditDistance.h"


namespace atp
{
namespace stats
{


/**
\brief This class computes the edit distance of equational logic
	statements.
	
\details This class is optimised in two ways: (i) it is specialised
	to the logic type so is more efficient, and (ii) caches
	information between calls.

\note The returned value is not a distance per se, as it is signed.
	It is more of a heuristic inspired by edit distance than any true
	distance metric.

\warning This class is **NOT** thread safe, please use a different
	version of this heuristic for each solver object.
*/
class ATP_STATS_API EquationalEditDistanceTracker :
	public IEditDistance
{
private:
	typedef logic::equational::Expression Expression;

	// mapping of hash codes to edit distances
	typedef std::map<size_t, float> EditDistMemoisation;

public:
	/**
	\param match_benefit A nonnegative number representing the
		DECREASE in edit distance PER DEPTH OF MATCHING

	\param unmatch_cost A nonnegative number representing the
		increase in edit distance when a pair of symbols are not
		matched.
	*/
	EquationalEditDistanceTracker(float match_benefit, float unmatch_cost);

	float edit_distance(const logic::IStatement& stmt1,
		const logic::IStatement& stmt2) override;

	std::vector<std::vector<float>> edit_distance(
		const logic::IStatementArray& stmtarr1,
		const logic::IStatementArray& stmtarr2) override;

	std::vector<std::vector<std::vector<float>>> sub_edit_distance(
		const logic::IStatementArray& stmtarr1,
		const logic::IStatementArray& stmtarr2) override;

private:
	/**
	\brief Helper function for computing edit distance between exprs

	\param depth The depth of these two expressions, where 0 is the
		depth of the root.

	\returns The cost that was calculated / cached
	*/
	float edit_distance(const Expression& expr1,
		const Expression& expr2, size_t depth);

	/**
	\brief Compute hash function of expression which is invariant to
		free variable renaming.
	*/
	size_t hash_expr(const Expression& expr) const;

	/**
	\brief Compute hash function of two expressions which is
		invariant to free variable renaming, and is also invairnat
		to switching the order of the two
	*/
	size_t hash_exprs(const Expression& expr1,
		const Expression& expr2) const;

private:
	const float m_match_benefit, m_unmatch_cost;
	EditDistMemoisation m_dists;
};


}  // namespace stats
}  // namespace atp


