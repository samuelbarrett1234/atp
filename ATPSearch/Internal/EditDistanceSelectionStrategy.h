#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a theorem selector strategy based on edit distance
	computations to the targets.

*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include <ATPStatsEditDistance.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/ISelectionStrategy.h"
#include "FixedSelectionStrategy.h"


namespace atp
{
namespace search
{


/**
\brief A basic theorem selector strategy which just picks
	randomly from the database to help in a proof.
*/
class ATP_SEARCH_API EditDistanceSelectionStrategy :
	public FixedSelectionStrategy
{
public:
	/**
	\param num_load The number of theorems to load from the database

	\param num_return The number of theorems to return from `done`,
		which will be a subset of the theorems loaded from the
		database (note that we could return less than this, as there
		are no guarantees of this many being present in the database
		in the first place.)

	\param symbol_mismatch_cost The edit distance cost for a mismatch
		of symbols, relative to the cost of a free variable
		substitution which is always fixed at 1.0

	\param symbol_match_benefit The corresponding benefit (decrease
		in edit distance) incurred when two symbols match

	\param weighting Higher means a statement is considered "better"
		if it is useful in lots of places; lower means a statement is
		considered "better" if it is very useful in a handful of
		places. Must be > 0.

	\pre symbol_mismatch_cost > 0 and symbol_match_benefit > 0 and
		weighting > 0

	\pre num_load >= num_return
	*/
	EditDistanceSelectionStrategy(const logic::LanguagePtr& p_lang,
		const logic::ModelContextPtr& p_ctx, size_t ctx_id,
		size_t num_load, size_t num_return, float symb_match_benefit,
		float symb_mismatch_cost, float weighting);

	void set_targets(
		const logic::StatementArrayPtr& p_targets) override;
	logic::StatementArrayPtr done() override;

private:
	const size_t m_num_thms_to_return;
	const stats::EditDistancePtr m_ed_dist;
	logic::StatementArrayPtr m_targets;
	const float m_weighting;
};


}  // namespace search
}  // namespace atp


