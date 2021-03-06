/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include "HMMConjectureModel.h"
#include "ATPStatsHMM.h"


namespace ublas = boost::numeric::ublas;


namespace atp
{
namespace core
{


// this function creates a matrix which computes the partial sums of
// a vector (which turns out to be a upper triangular matrix of
// all-1s).
// it has to be upper not lower because it's a stochastic matrix
// so the vectors multiply on the LHS
ublas::triangular_matrix<float, ublas::upper> psum_matrix(size_t n)
{
	ublas::triangular_matrix<float, ublas::upper> m(n, n);

	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j <= i; ++j)
			m(j, i) = 1.0f;

	return m;
}


HMMConjectureModel::HMMConjectureModel(logic::ModelContextPtr p_ctx,
	size_t num_states, float free_q,
	ublas::matrix<float> st_trans, ublas::matrix<float> st_obs,
	std::vector<size_t> symbs, size_t rand_seed, float smoothing,
	float decay) :
	m_ctx(std::move(p_ctx)),
	m_num_hidden_states(num_states),
	m_free_q(free_q), m_decay(decay),
	m_state(ublas::zero_vector<float>(num_states)),
	m_st_trans(std::move(st_trans)),
	m_st_obs(st_obs),
	m_st_obs_partial_sums(ublas::prod(st_obs,
		psum_matrix(m_symbs.size()))),
	m_symbs(std::move(symbs)),
	m_rand_device((unsigned int)rand_seed),
	m_unif01(0.0f, 1.0f), m_smoothing(smoothing),
	m_stmt_to_obs(m_ctx, m_symbs)
{
	ATP_CORE_PRECOND(m_ctx != nullptr);
	ATP_CORE_PRECOND(m_num_hidden_states > 0);
	ATP_CORE_PRECOND(free_q >= 0.0f);
	ATP_CORE_PRECOND(free_q <= 1.0f);
	ATP_CORE_PRECOND(decay >= 0.0f);
	ATP_CORE_PRECOND(decay <= 1.0f);
	ATP_CORE_PRECOND(smoothing >= 0.0f);
	ATP_CORE_PRECOND(m_st_trans.size1() == m_num_hidden_states);
	ATP_CORE_PRECOND(m_st_trans.size2() == m_num_hidden_states);
	ATP_CORE_PRECOND(m_st_obs_partial_sums.size1() ==
		m_num_hidden_states);
	ATP_CORE_PRECOND(m_st_obs_partial_sums.size2() ==
		m_symbs.size());
	// we're not going to check that m_symbs contains exactly all of
	// the symbol IDs contained in the model context, but this is
	// certainly a precondition

	reset_state();  // generates observation
}


void HMMConjectureModel::advance()
{
	m_state = boost::numeric::ublas::prod(m_state, m_st_trans);

#ifdef ATP_CORE_DEFENSIVE
	const float sum = ublas::sum(m_state);
	if (std::abs(sum - 1.0f) > 1.0e-4f)
	{
		ATP_CORE_LOG(warning) << "HMM conjecture model state "
			"distribution didn't sum to 1; sum was "
			<< sum;
	}
#endif

	generate_observation();
}


void HMMConjectureModel::train(
	const logic::StatementArrayPtr& p_stmts, size_t N)
{
	ATP_CORE_PRECOND(p_stmts != nullptr);
	if (N == 0)
		return;

	const auto observations = m_stmt_to_obs.convert(p_stmts);

	ublas::vector<float> initial_state = ublas::scalar_vector<float>(
		m_num_hidden_states, 1.0f / (float)m_num_hidden_states);

	ublas::matrix<float> obs_with_free = m_st_obs;
	obs_with_free.resize(m_num_hidden_states, m_symbs.size() + 1);

	// extend emission matrix with free variable observations
	for (size_t i = 0; i < m_num_hidden_states; ++i)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < m_symbs.size(); ++j)
			sum += obs_with_free(i, j);
		obs_with_free(i, m_symbs.size()) = 1.0f - sum;
	}

	stats::baum_welch(initial_state, m_st_trans, obs_with_free,
		observations, N, m_smoothing, m_decay);

	// extract back the portion of the matrix we care about
	m_st_obs = ublas::matrix_range<ublas::matrix<float>>(
		obs_with_free, ublas::range(0, m_num_hidden_states),
		ublas::range(0, m_symbs.size()));
}


void HMMConjectureModel::generate_observation()
{
	const auto obs_cum_probs = ublas::prod(
		m_state, m_st_obs_partial_sums);

#ifdef ATP_CORE_DEFENSIVE
	ATP_CORE_ASSERT(std::is_sorted(obs_cum_probs.cbegin(),
		obs_cum_probs.cend()));
#endif

#ifdef ATP_CORE_DEFENSIVE
	if (obs_cum_probs(m_symbs.size() - 1) > 1.0f)
	{
		ATP_CORE_LOG(warning) << "HMM conjecture model observation "
			"distribution didn't sum to 1; sum was "
			<< obs_cum_probs(m_symbs.size() - 1);
	}
	else if (std::abs(obs_cum_probs(m_symbs.size() - 1) - 1.0f) < 1.0e-6f)
	{
		ATP_CORE_LOG(warning) << "HMM conjecture model observation "
			"distribution summed to 1, excluding free variables, so "
			"it will be impossible (/incredibly unlikely) that a "
			"free variable is generated.";
	}
#endif

	// generate random probability
	const float r = m_unif01(m_rand_device);

	// find the least element in `obs_probs` which is >= r
	const auto observation = std::upper_bound(obs_cum_probs.cbegin(),
		obs_cum_probs.cend(), r);

	if (observation != obs_cum_probs.cend())
	{
		// we have generated a symbol as our current observation:
		m_cur_obs = m_symbs[std::distance(obs_cum_probs.cbegin(),
			observation)];
		
		ATP_CORE_ASSERT(m_ctx->is_defined(m_cur_obs));

		m_cur_obs_arity = m_ctx->symbol_arity(m_cur_obs);

		m_cur_obs_is_free = false;
	}
	else
	{
		// we have generated a free variable!

		m_cur_obs = 0;

		// impose some silly limit to guarantee termination:
		static const size_t max_free_id = 1000;

		// draw from geometric distribution:
		while (m_cur_obs < max_free_id &&
			m_unif01(m_rand_device) <= m_free_q)
			++m_cur_obs;

		m_cur_obs_arity = 0;
		m_cur_obs_is_free = true;
	}
}


void HMMConjectureModel::estimate_free_q(
	const logic::StatementArrayPtr& p_stmts)
{
	// num_occ_id[i] is the number of occurrences of a free variable
	// with ID = i
	const std::vector<size_t> num_occ_id = m_stmt_to_obs.count_free_ids(
		p_stmts);

	size_t sum = 0;
	for (size_t x : num_occ_id)
		sum += x;

	if (sum == 0)
		return;  // avoid division by zero

	// denominator of the MLE for free_q
	float mle_denom = 0.0f;
	for (size_t i = 0; i < num_occ_id.size(); ++i)
	{
		mle_denom += static_cast<float>((i + 1) * num_occ_id[i]);
	}

	m_free_q = 1.0f - (float)sum / mle_denom;
}


}  // namespace core
}  // namespace atp


