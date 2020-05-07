/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/matrix_proxy.hpp>
#include "HMMConjectureTrainer.h"


namespace ublas = boost::numeric::ublas;
typedef ublas::matrix<float> Matrix;
typedef ublas::matrix_range<Matrix> MatrixRange;
typedef ublas::matrix_column<Matrix> MatrixCol;
typedef ublas::matrix_row<Matrix> MatrixRow;


namespace atp
{
namespace core
{


HMMConjectureTrainer::HMMConjectureTrainer(
	const logic::LanguagePtr& p_lang, size_t ctx_id,
	const logic::ModelContextPtr& p_ctx,
	Matrix st_trans_init,
	Matrix st_obs_init,
	std::vector<size_t> symbols, float smoothing) :
	m_lang(p_lang), m_ctx_id(m_ctx_id), m_ctx(m_ctx),
	m_st_trans(std::move(st_trans_init)),
	m_st_obs(std::move(st_obs_init)),
	m_smoothing(smoothing), m_symbols(std::move(symbols)),
	m_num_states(m_st_trans.size1()),
	m_stmt_to_obs(m_lang, m_ctx, m_symbols)
{
	ATP_CORE_PRECOND(m_symbols.size() > 0);
	ATP_CORE_PRECOND(m_lang != nullptr);
	ATP_CORE_PRECOND(m_ctx != nullptr);
	ATP_CORE_PRECOND(m_num_states > 0);
	ATP_CORE_PRECOND(m_st_trans.size2() == m_num_states);
	ATP_CORE_PRECOND(m_st_obs.size2() == m_num_states);
	ATP_CORE_PRECOND(m_st_obs.size1() == m_symbols.size());
	ATP_CORE_PRECOND(m_smoothing >= 0.0f);

	// adjoin an extra row of observations, for free variables:
	m_st_obs.resize(m_st_obs.size1() + 1, m_st_obs.size2());
	for (size_t i = 0; i < m_num_states; ++i)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < m_symbols.size(); ++j)
		{
			sum += m_st_obs(j, i);
		}
		m_st_obs(m_symbols.size(), i) = 1.0f - sum;
	}
}


Matrix HMMConjectureTrainer::get_obs_mat()
{
	return MatrixRange(m_st_obs,
		ublas::range(0, m_symbols.size()),
		ublas::range(0, m_num_states));
}


void HMMConjectureTrainer::train(
	const logic::StatementArrayPtr& p_stmts, size_t N)
{
	const auto observations = m_stmt_to_obs.convert(p_stmts);

	baum_welch_update(observations, N);
}


Matrix HMMConjectureTrainer::forward(
	const std::vector<size_t>& obs_seq)
{
	Matrix A(obs_seq.size() + 1,
		m_num_states);

	// input initial state distribution:
	for (size_t i = 0; i < m_num_states; ++i)
	{
		A(0, i) = 1.0f / (float)m_num_states;
	}

	// now go forwards
	for (size_t t = 1; t <= obs_seq.size(); ++t)
	{
		ATP_CORE_ASSERT(obs_seq[t - 1] < m_st_obs.size1());

		MatrixCol last_t(A, t - 1);
		MatrixCol cur_t(A, t);
		MatrixRow obs_probs(
			m_st_obs, obs_seq[t - 1]);

		// advance via state transition
		cur_t = ublas::prod(ublas::trans(m_st_trans), last_t);

		// multiply by observation probabilities
		cur_t = ublas::element_prod(cur_t, obs_probs);
	}

	return A;
}


Matrix HMMConjectureTrainer::backward(
	const std::vector<size_t>& obs_seq)
{
	Matrix B(obs_seq.size() + 1,
		m_num_states);

	// initialise:
	for (size_t i = 0; i < m_num_states; ++i)
	{
		B(obs_seq.size(), i) = 1.0f;
	}

	// now go backwards
	for (size_t _t = obs_seq.size(); _t > 0; --_t)
	{
		const size_t t = _t - 1;

		MatrixCol last_t(B, t + 1);
		MatrixCol cur_t(B, t);
		MatrixRow obs_probs(
			m_st_obs, obs_seq[t - 1]);

		// multiply by observation probabilities
		cur_t = ublas::element_prod(last_t, obs_probs);

		// go backwards via state transition
		cur_t = ublas::prod(m_st_trans, cur_t);
	}

	return B;
}


void HMMConjectureTrainer::baum_welch_update(
	const std::vector<std::vector<size_t>>& obs_seqs, size_t N)
{
	ublas::vector<float> ones =
		ublas::scalar_vector(m_num_states, 1.0f);

	// for each training epoch...
	for (size_t epoch = 0; epoch < N; ++epoch)
	{
		// for each training data point...
		for (size_t d = 0; d < obs_seqs.size(); ++d)
		{
			const auto& obs_seq = obs_seqs[d];

			Matrix A = forward(obs_seq);
			Matrix B = backward(obs_seq);
			Matrix AB = ublas::element_prod(A, B);

			Matrix C(obs_seq.size(), m_num_states);

			for (size_t t = 0; t < obs_seq.size(); ++t)
			{
				MatrixCol C_t(C, t),
					AB_t(AB, t + 1);

				C_t = AB_t;

				// add smoothing parameter
				C_t += ublas::scalar_vector<float>(m_num_states,
					m_smoothing);

				C_t /= ublas::sum(AB_t);  // normalise
			}

			std::vector<Matrix> Ds(obs_seq.size(),
				Matrix(m_num_states, m_num_states));

			for (size_t t = 0; t < obs_seq.size(); ++t)
			{
				MatrixCol A_t(A, t + 1), B_t(B, t + 1);
				MatrixRow obs_probs(
					m_st_obs, obs_seq[t - 1]);

				Ds[t] = ublas::element_prod(m_st_trans,
					ublas::outer_prod(A_t,
						ublas::element_prod(B_t,
							obs_probs)));

				// add smoothing parameter
				Ds[t] += ublas::scalar_matrix<float>(Ds[t].size1(),
					Ds[t].size2(), m_smoothing);
				
				// normalise
				Ds[t] /= ublas::sum(
					ublas::prod(Ds[t], ones));
			}

			for (size_t i = 0; i < m_num_states; ++i)
			{
				for (size_t j = 0; j < m_num_states; ++j)
				{
					float dsum = 0.0f, csum = 0.0f;
					for (size_t t = 0; t < obs_seq.size() - 1; ++t)
					{
						dsum += Ds[t](i, j);
						csum += C(t, i);
					}
					m_st_trans(i, j) = dsum / csum;
				}

				for (size_t j = 0; j < m_symbols.size() + 1; ++j)
				{
					float csum = 0.0f, csum_jth_obs = 0.0f;
					for (size_t t = 0; t < obs_seq.size() - 1; ++t)
					{
						csum += C(t, i);
						if (obs_seq[t] == j)
							csum_jth_obs += C(t, i);
						else
							csum_jth_obs += m_smoothing;
					}
					m_st_obs(j, i) = csum_jth_obs / csum;
				}
			}
		}
	}
}


}  // namespace core
}  // namespace atp


