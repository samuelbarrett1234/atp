#pragma once


/**
\file

\author Samuel Barrett

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class ATP_DATABASE_API SQLiteSaveProofResultsQryBder :
	public ISaveProofResultsQryBder
{
public:
	std::string build() override;
	inline ISaveProofResultsQryBder* set_context(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		ATP_DATABASE_PRECOND(p_ctx != nullptr);
		m_ctx_id = ctx_id;
		m_ctx = p_ctx;
		return this;
	}
	inline ISaveProofResultsQryBder* set_search_settings(
		size_t ss_id) override
	{
		m_ss_id = ss_id;
		return this;
	}
	inline ISaveProofResultsQryBder* reset() override
	{
		m_ctx_id = boost::none;
		m_ss_id = boost::none;
		m_size = boost::none;
		m_ctx = boost::none;
		m_targets = boost::none;
		m_proof_states = boost::none;
		m_times = boost::none;
		m_max_mems = boost::none;
		m_num_exps = boost::none;
		return this;
	}
	inline ISaveProofResultsQryBder* add_target_thms(
		const logic::StatementArrayPtr& p_targets) override
	{
		ATP_DATABASE_PRECOND(p_targets != nullptr);
		set_or_check_size(p_targets->size());
		m_targets = p_targets;
		return this;
	}
	inline ISaveProofResultsQryBder* add_proof_states(
		const std::vector<logic::ProofStatePtr>& proof_states) override
	{
		set_or_check_size(proof_states.size());
		m_proof_states = proof_states;
		return this;
	}
	inline ISaveProofResultsQryBder* add_proof_times(
		const std::vector<float>& proof_times) override
	{
		set_or_check_size(proof_times.size());
		m_times = proof_times;
		return this;
	}
	inline ISaveProofResultsQryBder* add_max_mem_usages(
		const std::vector<size_t>& max_mem_usages) override
	{
		set_or_check_size(max_mem_usages.size());
		m_max_mems = max_mem_usages;
		return this;
	}
	inline ISaveProofResultsQryBder* add_num_node_expansions(
		const std::vector<size_t>& num_node_expansions) override
	{
		set_or_check_size(num_node_expansions.size());
		m_num_exps = num_node_expansions;
		return this;
	}

private:
	inline void set_or_check_size(size_t size)
	{
		if (m_size.has_value())
		{
			ATP_DATABASE_PRECOND(*m_size == size);
		}
		else
		{
			m_size = size;
		}
	}

private:
	boost::optional<size_t> m_ctx_id, m_ss_id, m_size;
	boost::optional<logic::ModelContextPtr> m_ctx;
	boost::optional<logic::StatementArrayPtr> m_targets;
	boost::optional<
		std::vector<logic::ProofStatePtr>> m_proof_states;
	boost::optional<std::vector<float>> m_times;
	boost::optional<std::vector<size_t>> m_max_mems, m_num_exps;
};


}  // namespace db
}  // namespace atp


