/**
\file

\author Samuel Barrett

*/


#include "EditDistanceSelectionStrategy.h"
#include "../ATPSearchLog.h"
#include <queue>


namespace atp
{
namespace search
{


EditDistanceSelectionStrategy::EditDistanceSelectionStrategy(
	const logic::LanguagePtr& p_lang,
	const logic::ModelContextPtr& p_ctx, size_t ctx_id,
	size_t num_load, size_t num_return, float symb_match_benefit,
	float symb_mismatch_cost, float weighting) :
	FixedSelectionStrategy(p_lang, p_ctx, ctx_id, num_load),
	m_num_thms_to_return(num_return), m_weighting(weighting),
	m_ed_dist(stats::create_edit_dist(logic::LangType::EQUATIONAL_LOGIC,  // TEMP!!
		symb_match_benefit, symb_mismatch_cost))
{
	ATP_SEARCH_PRECOND(m_weighting > 0.0f);
}


void EditDistanceSelectionStrategy::set_targets(const logic::StatementArrayPtr& p_targets)
{
	ATP_SEARCH_PRECOND(p_targets != nullptr);
	FixedSelectionStrategy::set_targets(p_targets);
	m_targets = p_targets;
}


logic::StatementArrayPtr EditDistanceSelectionStrategy::done()
{
	ATP_SEARCH_PRECOND(m_targets != nullptr);

	auto stmts = FixedSelectionStrategy::done();

	// handle this rare case (but a possible case)
	if (stmts == nullptr)
	{
		ATP_SEARCH_LOG(error) << "Oops: helper theorem loading "
			"failed...";
		return nullptr;
	}

	// if there weren't enough statements in the database in the
	// first place, just return the whole lot, without performing
	// any heuristic computation
	if (stmts->size() <= m_num_thms_to_return)
		return stmts;

	// now select subset of theorems:

		// compute edit distance between all the pairs
	const auto distance_matrix =
		m_ed_dist->sub_edit_distance(
			*m_targets, *stmts);

	// pick best members:
	std::vector<float> utilities;
	utilities.resize(stmts->size(), 0.0f);
	for (size_t i = 0; i < m_targets->size(); ++i)
	{
		for (size_t j = 0; j < stmts->size(); ++j)
		{
			const float best = *std::min_element(
				distance_matrix[i][j].begin(),
				distance_matrix[i][j].end());

			// higher distance means worse, so use negative
			// (note that the entries may be negative, as edit
			// distance isn't a distance metric, rather just a
			// distance-inspired heuristic)
			float s = 0.0f;
			for (size_t k = 0; k < distance_matrix[i][j].size(); ++k)
			{
				s += -std::powf(distance_matrix[i][j][k] - best,
					m_weighting);
			}
			// divide by size to normalise it a bit
			utilities[j] += -best + s / (float)distance_matrix[i][j].size();
		}
	}

	// obtain top m_setup_data.settings.num_helper_thms elements
	// using a priority queue
	std::priority_queue<std::pair<float, size_t>> q;
	ATP_SEARCH_ASSERT(utilities.size() == stmts->size());
	for (size_t i = 0; i < utilities.size(); ++i)
	{
		q.emplace(utilities[i], i);
	}

	// extract out the best elements
	std::vector<logic::StatementArrayPtr> singletons;
	singletons.reserve(m_num_thms_to_return);
	for (size_t i = 0; i < m_num_thms_to_return;
		++i)
	{
		auto [util, k] = q.top();
		q.pop();

		ATP_SEARCH_LOG(trace) << "Selected \"" <<
			stmts->at(k).to_str() << "\" as a good statement to "
			"help prove the target theorems (utility was "
			<< util << ").";

		singletons.emplace_back(stmts->slice(k, k + 1));
	}
	return logic::concat(singletons);
}


}  // namespace search
}  // namespace atp


