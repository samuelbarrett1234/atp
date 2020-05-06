/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/triangular.hpp>
#include "HMMConjectureModel.h"


namespace ub = boost::numeric::ublas;


namespace atp
{
namespace core
{


// this function creates a matrix which computes the partial sums of
// a vector (which turns out to be a lower triangular matrix of
// all-1s).
ub::triangular_matrix<float, ub::lower> psum_matrix(size_t n)
{
	ub::triangular_matrix<float, ub::lower> m(n, n);

	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j <= i; ++j)
			m(i, j) = 1.0f;

	return m;
}


HMMConjectureModel::HMMConjectureModel(logic::ModelContextPtr p_ctx,
	size_t num_states, float free_q,
	ub::matrix<float> st_trans, ub::matrix<float> st_obs,
	std::vector<size_t> symbs, size_t rand_seed) :
	m_ctx(std::move(p_ctx)),
	m_num_hidden_states(num_states),
	m_free_q(free_q),
	m_state(ub::zero_vector<float>(num_states)),
	m_st_trans(std::move(st_trans)),
	m_st_obs_partial_sums(ub::prod(psum_matrix(m_symbs.size()),
		st_obs)),
	m_symbs(std::move(symbs)),
	m_rand_device((unsigned int)rand_seed),
	m_unif01(0.0f, 1.0f)
{
	ATP_CORE_PRECOND(m_ctx != nullptr);
	ATP_CORE_PRECOND(m_num_hidden_states > 0);
	ATP_CORE_PRECOND(free_q >= 0.0f);
	ATP_CORE_PRECOND(free_q <= 1.0f);
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


void HMMConjectureModel::generate_observation()
{
	const auto obs_probs = ub::prod(m_st_obs_partial_sums, m_state);

	// generate random probability
	const float r = m_unif01(m_rand_device);

	// find the least element in `obs_probs` which is >= r
	const auto observation = std::upper_bound(obs_probs.cbegin(),
		obs_probs.cend(), r);

	if (observation != obs_probs.cend())
	{
		// we have generated a symbol as our current observation:
		m_cur_obs = m_symbs[std::distance(obs_probs.cbegin(),
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


}  // namespace core
}  // namespace atp


