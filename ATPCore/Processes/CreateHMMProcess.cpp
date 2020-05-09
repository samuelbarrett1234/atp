/**
\file

\author Samuel Barrett

*/


#include <chrono>
#include <random>
#include <vector>
#include <boost/math/distributions/beta.hpp>
#include "CreateHMMProcess.h"
#include "QueryProcess.h"


namespace atp
{
namespace core
{


class CreateHMMProcess :
	public QueryProcess
{
public:
	CreateHMMProcess(
		logic::LanguagePtr p_lang, size_t ctx_id,
		logic::ModelContextPtr p_ctx, db::DatabasePtr p_db,
		size_t num_hidden_states, size_t model_id) :
		QueryProcess(p_db), m_ctx_id(ctx_id), m_rand_eng((unsigned int)cur_time()),
		m_num_states(num_hidden_states), m_unif01(0.0f, 1.0f),
		m_db(p_db), m_ctx(p_ctx), m_model_id(model_id)
	{
		ATP_CORE_LOG(trace) << "Starting up process to create new "
			"HMM...";

		auto consts = p_ctx->all_constant_symbol_ids();
		auto funcs = p_ctx->all_function_symbol_ids();
		m_symbols.insert(m_symbols.end(), consts.begin(),
			consts.end());
		m_symbols.insert(m_symbols.end(), funcs.begin(),
			funcs.end());

		m_free_q = generate_beta(0.25f, 0.01f);

		for (size_t i = 0; i < m_num_states; ++i)
		{
			m_st_trans.emplace_back(generate_dist(
				1.0f / (float)m_num_states, 0.01f, m_num_states));

			// generate one extra, for free variables
			auto obs_dist = generate_dist(
				1.0f / (float)m_symbols.size(), 0.01f,
				m_symbols.size() + 1);

			obs_dist.pop_back();
			m_st_obs.emplace_back(std::move(obs_dist));
		}
	}

private:
	static unsigned int cur_time()
	{
		return (unsigned int)std::chrono::high_resolution_clock
			::now().time_since_epoch().count();
	}

	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(info) << "Creating HMM model for context with "
			"ID " << m_ctx_id << " using " << m_num_states <<
			" states; model ID = " << m_model_id << ".";

		auto _p_bder = m_db->create_query_builder(
			db::QueryBuilderType::SAVE_HMM_CONJ_PARAMS);

		auto p_bder = dynamic_cast<db::ISaveHmmConjModelParams*>(
			_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_ctx(m_ctx_id, m_ctx)
			->set_num_states(m_num_states)
			->set_free_q(m_free_q)
			->set_model_id(m_model_id);

		for (size_t i = 0; i < m_num_states; ++i)
		{
			// add symbols
			for (size_t j = 0; j < m_num_states; ++j)
			{
				p_bder->add_state_trans(i, j, m_st_trans[i][j]);
			}

			// add observations
			for (size_t j = 0; j < m_symbols.size(); ++j)
			{
				p_bder->add_observation(i, m_symbols[j],
					m_st_obs[i][j]);
			}
		}

		return _p_bder;
	}

	void on_finished() override
	{
		ATP_CORE_LOG(info) << "Finished creating new HMM model.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Failed to create HMM model.";
	}

	float generate_beta(float mean, float var)
	{
		const float r = m_unif01(m_rand_eng);

		const float alpha = boost::math::beta_distribution<float>
			::find_alpha(mean, var);
		const float beta = boost::math::beta_distribution<float>
			::find_beta(mean, var);

		boost::math::beta_distribution<float> dist(alpha, beta);

		return boost::math::quantile(dist, r);
	}

	std::vector<float> generate_dist(float mean, float var, size_t N)
	{
		ATP_CORE_PRECOND(N > 0);

		std::vector<float> result;
		result.reserve(N);
		float sum = 0.0f;

		for (size_t i = 0; i < N - 1; ++i)
		{
			const float x = generate_beta(mean, var);

			result.push_back((1.0f - sum) * x);
			sum += result.back();
		}

		// last element is fixed
		result.push_back(1.0f - sum);

		// shuffle to make sure we're impartial to the order
		std::shuffle(result.begin(), result.end(), m_rand_eng);

		return result;
	}

private:
	logic::ModelContextPtr m_ctx;
	db::DatabasePtr m_db;
	std::mt19937 m_rand_eng;
	std::uniform_real_distribution<float> m_unif01;
	std::vector<size_t> m_symbols;
	std::vector<std::vector<float>> m_st_trans, m_st_obs;
	float m_free_q;
	const size_t m_num_states, m_ctx_id, m_model_id;
};


ATP_CORE_API ProcessPtr create_hmm_process(
	logic::LanguagePtr p_lang, size_t ctx_id,
	logic::ModelContextPtr p_ctx, db::DatabasePtr p_db,
	size_t num_hidden_states, size_t model_id)
{
	// check inputs
	ATP_CORE_PRECOND(p_lang != nullptr);
	ATP_CORE_PRECOND(p_ctx != nullptr);
	ATP_CORE_PRECOND(p_db != nullptr);
	ATP_CORE_PRECOND(num_hidden_states > 0);

	return std::make_shared<CreateHMMProcess>(p_lang, ctx_id,
		p_ctx, p_db, num_hidden_states, model_id);
}


}  // namespace core
}  // namespace atp


