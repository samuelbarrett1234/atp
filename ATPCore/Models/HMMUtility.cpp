/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/matrix_proxy.hpp>
#include "HMMUtility.h"


namespace ublas = boost::numeric::ublas;
typedef ublas::matrix<float> Matrix;
typedef ublas::matrix_column<Matrix> MatrixCol;
typedef ublas::matrix_row<Matrix> MatrixRow;


namespace atp
{
namespace core
{
namespace hmm
{


Matrix forward(
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<size_t>& obs_seq)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size1();

	Matrix A(obs_seq.size() + 1,
		num_states);

	// input initial state distribution:
	for (size_t i = 0; i < num_states; ++i)
	{
		A(0, i) = 1.0f / (float)num_states;
	}

	// now go forwards
	for (size_t t = 1; t <= obs_seq.size(); ++t)
	{
		ATP_CORE_ASSERT(obs_seq[t - 1] < st_obs.size1());

		MatrixCol last_t(A, t - 1);
		MatrixCol cur_t(A, t);
		MatrixRow obs_probs(
			st_obs, obs_seq[t - 1]);

		// advance via state transition
		cur_t = ublas::prod(ublas::trans(st_trans), last_t);

		// multiply by observation probabilities
		cur_t = ublas::element_prod(cur_t, obs_probs);
	}

	return A;
}


Matrix backward(
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<size_t>& obs_seq)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size1();

	Matrix B(obs_seq.size() + 1,
		num_states);

	// initialise:
	for (size_t i = 0; i < num_states; ++i)
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
			st_obs, obs_seq[t - 1]);

		// multiply by observation probabilities
		cur_t = ublas::element_prod(last_t, obs_probs);

		// go backwards via state transition
		cur_t = ublas::prod(st_trans, cur_t);
	}

	return B;
}


void baum_welch(
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<std::vector<size_t>>& obs_seqs,
	size_t num_epochs, float smoothing)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size1();

	ublas::vector<float> ones =
		ublas::scalar_vector(num_states, 1.0f);

	// for each training epoch...
	for (size_t epoch = 0; epoch < num_epochs; ++epoch)
	{
		// for each training data point...
		for (size_t d = 0; d < obs_seqs.size(); ++d)
		{
			const auto& obs_seq = obs_seqs[d];

			Matrix A = forward(st_trans, st_obs, obs_seq);
			Matrix B = backward(st_trans, st_obs, obs_seq);
			Matrix AB = ublas::element_prod(A, B);

			Matrix C(obs_seq.size(), num_states);

			for (size_t t = 0; t < obs_seq.size(); ++t)
			{
				MatrixCol C_t(C, t),
					AB_t(AB, t + 1);

				C_t = AB_t;

				// add smoothing parameter
				C_t += ublas::scalar_vector<float>(num_states,
					smoothing);

				C_t /= ublas::sum(AB_t);  // normalise
			}

			std::vector<Matrix> Ds(obs_seq.size(),
				Matrix(num_states, num_states));

			for (size_t t = 0; t < obs_seq.size(); ++t)
			{
				MatrixCol A_t(A, t + 1), B_t(B, t + 1);
				MatrixRow obs_probs(
					st_obs, obs_seq[t - 1]);

				Ds[t] = ublas::element_prod(st_trans,
					ublas::outer_prod(A_t,
						ublas::element_prod(B_t,
							obs_probs)));

				// add smoothing parameter
				Ds[t] += ublas::scalar_matrix<float>(Ds[t].size1(),
					Ds[t].size2(), smoothing);

				// normalise
				Ds[t] /= ublas::sum(
					ublas::prod(Ds[t], ones));
			}

			for (size_t i = 0; i < num_states; ++i)
			{
				for (size_t j = 0; j < num_states; ++j)
				{
					float dsum = 0.0f, csum = 0.0f;
					for (size_t t = 0; t < obs_seq.size() - 1; ++t)
					{
						dsum += Ds[t](i, j);
						csum += C(t, i);
					}
					st_trans(i, j) = dsum / csum;
				}

				for (size_t j = 0; j < num_obs + 1; ++j)
				{
					float csum = 0.0f, csum_jth_obs = 0.0f;
					for (size_t t = 0; t < obs_seq.size() - 1; ++t)
					{
						csum += C(t, i);
						if (obs_seq[t] == j)
							csum_jth_obs += C(t, i);
						else
							csum_jth_obs += smoothing;
					}
					st_obs(j, i) = csum_jth_obs / csum;
				}
			}
		}
	}
}


}  // namespace hmm
}  // namespace core
}  // namespace atp


