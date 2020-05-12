#pragma once


/**
\file

\author Samuel Barrett

\brief This file contains edit distance computations specialised to
	equational logic.
*/


#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <ATPLogic.h>
#include <Internal/Equational/Expression.h>
#include "../ATPStatsAPI.h"
#include "EditDistanceUtility.h"
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
*/
class ATP_STATS_API EquationalEditDistanceTracker :
	public IEditDistance
{
private:
	typedef logic::equational::Expression Expression;

	/**
	\brief A hash function for equational expressions
	*/
	struct ExprHashFunc
	{
		size_t operator()(const Expression& expr) const;
	};

	/**
	\brief A hash function for pairs of equational expressions
	*/
	struct ExprPairHashFunc
	{
		ExprHashFunc ehf;

		size_t operator()(const std::pair<Expression,
			Expression>& p) const;
	};

	/**
	\brief An equality function for pairs of equational expressions,
		needed for the unordered map which stores them.

	\note Edit distance doesn't involve free variable IDs, so here we
		only need to check equivalence! Also, distance is symmetric,
		so check equivalence for the pairs in both directions.
	*/
	struct ExprPairEqFunc
	{
		bool operator()(const std::pair<Expression, Expression>& a,
			const std::pair<Expression, Expression>& b) const;
	};

	typedef std::unordered_map<std::pair<Expression, Expression>,
		float, ExprPairHashFunc, ExprPairEqFunc> EditDistMemoisation;

public:
	EquationalEditDistanceTracker(EditDistSubCosts sub_costs);

	float edit_distance(const logic::IStatement& stmt1,
		const logic::IStatement& stmt2) override;

	std::vector<std::vector<float>> edit_distance(
		const logic::IStatementArray& stmtarr1,
		const logic::IStatementArray& stmtarr2) override;

private:
	/**
	\brief Helper function for computing edit distance between exprs

	\details This function is still thread safe.
	*/
	float edit_distance(const Expression& expr1,
		const Expression& expr2);

	/**
	\brief This function ensures the edit distance of the two given
		expressions is present in the m_dists cache

	\param lock This is a lock which should be initialised
		with read-only access, and this function may attempt to
		upgrade it. It is assumed to be locked.
	*/
	void ensure_is_computed(const Expression& expr1,
		const Expression& expr2,
		boost::shared_lock<boost::shared_mutex>& lock);

private:
	// shared_mutex : multiple readers, at most one writer
	// manages access to the two objects below
	boost::shared_mutex m_mutex;

	EditDistSubCosts m_sub_costs;
	EditDistMemoisation m_dists;
};


}  // namespace stats
}  // namespace atp


