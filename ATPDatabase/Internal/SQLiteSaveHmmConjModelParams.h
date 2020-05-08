#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a class for building queries which save HMM model
	parameters to the database.
*/


#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class ATP_DATABASE_API SQLiteSaveHmmConjModelParams :
	public ISaveHmmConjModelParams
{
public:
	std::string build() override;
	inline ISaveHmmConjModelParams* reset() override
	{
		m_ctx_id.reset();
		m_ctx.reset();
		m_model_id.reset();
		m_num_states.reset();
		m_free_q.reset();
		m_st_trans.clear();
		m_st_obs.clear();
		return this;
	}
	inline ISaveHmmConjModelParams* set_model_id(size_t mid) override
	{
		m_model_id = mid;
		return this;
	}
	inline ISaveHmmConjModelParams* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		ATP_DATABASE_PRECOND(p_ctx != nullptr);

		m_ctx_id = ctx_id;
		m_ctx = p_ctx;

		return this;
	}
	inline ISaveHmmConjModelParams* set_num_states(size_t n) override
	{
		ATP_DATABASE_PRECOND(n > 0);
		m_num_states = n;
		return this;
	}
	inline ISaveHmmConjModelParams* set_free_q(float q) override
	{
		ATP_DATABASE_PRECOND(q >= 0.0f);
		ATP_DATABASE_PRECOND(q <= 1.0f);

		m_free_q = q;
		return this;
	}
	inline ISaveHmmConjModelParams* add_state_trans(size_t pre,
		size_t post, float prob) override
	{
		ATP_DATABASE_PRECOND(prob >= 0.0f);
		ATP_DATABASE_PRECOND(prob <= 1.0f);

		m_st_trans.emplace_back(pre, post, prob);
		return this;
	}
	inline ISaveHmmConjModelParams* add_observation(size_t state,
		size_t symb_id, float prob) override
	{
		ATP_DATABASE_PRECOND(m_ctx != nullptr);
		ATP_DATABASE_PRECOND(m_ctx->is_defined(symb_id));
		ATP_DATABASE_PRECOND(prob >= 0.0f);
		ATP_DATABASE_PRECOND(prob <= 1.0f);

		m_st_obs.emplace_back(state,
			m_ctx->symbol_name(symb_id), prob);
		return this;
	}

private:
	logic::ModelContextPtr m_ctx;
	boost::optional<size_t> m_ctx_id,
		m_model_id, m_num_states;
	boost::optional<float> m_free_q;
	std::vector<boost::tuple<size_t, size_t, float>> m_st_trans;
	std::vector<boost::tuple<size_t, std::string, float>> m_st_obs;
};


}  // namespace db
}  // namespace atp


