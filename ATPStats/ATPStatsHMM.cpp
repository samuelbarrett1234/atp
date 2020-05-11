/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/matrix_proxy.hpp>
#include "ATPStatsHMM.h"


namespace ublas = boost::numeric::ublas;
typedef ublas::vector<float> Vector;
typedef ublas::matrix<float> Matrix;
typedef ublas::matrix_column<Matrix> MatrixCol;
typedef ublas::matrix_row<Matrix> MatrixRow;


namespace atp
{
namespace stats
{


Matrix forward(
	Vector& initial_state,
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<size_t>& obs_seq)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size2();
	ATP_STATS_PRECOND(initial_state.size() == num_states);
	ATP_STATS_PRECOND(st_trans.size2() == num_states);
	ATP_STATS_PRECOND(st_obs.size1() == num_states);

	Matrix A(obs_seq.size() + 1,
		num_states);

	// input initial state distribution:
	MatrixRow(A, 0) = initial_state;

	// now go forwards
	for (size_t t = 1; t <= obs_seq.size(); ++t)
	{
		ATP_STATS_ASSERT(obs_seq[t - 1] < num_obs);

		MatrixRow last_t(A, t - 1);
		MatrixRow cur_t(A, t);
		MatrixCol obs_probs(
			st_obs, obs_seq[t - 1]);

		cur_t = ublas::element_prod(
			// advance via state transition
			ublas::prod(last_t, st_trans),
			// multiply by observation probabilities
			obs_probs);
	}

	return A;
}


Matrix backward(
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<size_t>& obs_seq)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size2();
	ATP_STATS_PRECOND(st_trans.size2() == num_states);
	ATP_STATS_PRECOND(st_obs.size1() == num_states);

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

		ATP_STATS_ASSERT(obs_seq[t] < num_obs);

		MatrixRow last_t(B, t + 1);
		MatrixRow cur_t(B, t);
		MatrixCol obs_probs(
			st_obs, obs_seq[t]);

		cur_t = ublas::prod(
			// multiply by observation probabilities
			ublas::element_prod(last_t, obs_probs),
			// go backwards via state transition (hence multiply
			// by the transpose)
			ublas::trans(st_trans));
	}

	return B;
}


void baum_welch(
	Vector& initial_state,
	Matrix& st_trans,
	Matrix& st_obs,
	const std::vector<std::vector<size_t>>& obs_seqs,
	size_t num_epochs, float smoothing, float decay)
{
	const size_t num_states = st_trans.size1();
	const size_t num_obs = st_obs.size2();
	ATP_STATS_PRECOND(initial_state.size() == num_states);
	ATP_STATS_PRECOND(st_trans.size2() == num_states);
	ATP_STATS_PRECOND(st_obs.size1() == num_states);

	ublas::vector<float> ones =
		ublas::scalar_vector(num_states, 1.0f);

	// for each training epoch...
	for (size_t epoch = 0; epoch < num_epochs; ++epoch)
	{
		std::vector<Matrix> Cs;
		Cs.reserve(obs_seqs.size());

		std::vector<std::vector<Matrix>> Dss;
		Dss.reserve(obs_seqs.size());

		// compute C and Ds for each data point
		for (size_t d = 0; d < obs_seqs.size(); ++d)
		{
			const auto& obs_seq = obs_seqs[d];

			// sequences of 0 or 1 observations give us no
			// information, so skip them
			if (obs_seq.size() <= 1)
			{
				continue;
			}

			// for good explanations of A and B, check the comments
			// for the postconditions of `forward` and `backward`
			Matrix A = forward(initial_state,
				st_trans, st_obs, obs_seq);
			Matrix B = backward(st_trans, st_obs, obs_seq);
			Matrix AB = ublas::element_prod(A, B);

			// the matrix C corresponds to the forward-backward
			// algorithm, and C(t, i) represents the probability
			// of being in state i at time t given ALL of the data.
			Cs.emplace_back(obs_seq.size() + 1, num_states);
			Matrix& C = Cs.back();

			for (size_t t = 0; t <= obs_seq.size(); ++t)
			{
				MatrixRow C_t(C, t),
					AB_t(AB, t);

				C_t = AB_t;

				// add smoothing parameter
				C_t += ublas::scalar_vector<float>(num_states,
					smoothing);

				C_t /= ublas::sum(C_t);  // normalise
			}

			// This is a substitute for a rank-3 tensor
			// Ds[t](i, j) represents the probability that the system
			// jumped from state i to state j at time t.
			Dss.emplace_back(obs_seq.size(),
				Matrix(num_states, num_states));
			auto& Ds = Dss.back();

			for (size_t t = 0; t < obs_seq.size(); ++t)
			{
				MatrixRow A_t(A, t), B_t(B, t + 1);
				MatrixCol obs_probs(
					st_obs, obs_seq[t]);

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
		}

		// given Cs and Dss, estimate the state transition and
		// observation matrices!
		for (size_t i = 0; i < num_states; ++i)
		{
			// estimate state transitions
			for (size_t j = 0; j < num_states; ++j)
			{
				float newparam = 0.0f;

				for (size_t d = 0; d < obs_seqs.size(); ++d)
				{
					auto& C = Cs[d];
					auto& Ds = Dss[d];
					const auto& obs_seq = obs_seqs[d];

					const float csum_0_to_Tminus1 = ublas::sum(
						MatrixCol(C, i)) - C(obs_seq.size(), i);

					ATP_STATS_ASSERT(csum_0_to_Tminus1 != 0.0f);

					float dsum = 0.0f;
					for (size_t t = 0; t < obs_seq.size(); ++t)
					{
						dsum += Ds[t](i, j);
					}
					newparam += dsum / csum_0_to_Tminus1;
				}

				// average over all data points, and apply decay:
				st_trans(i, j) = (1.0f - decay) * st_trans(i, j) +
					decay * newparam / (float)obs_seqs.size();
			}

			// normalise, just in case
			MatrixRow(st_trans, i) /= ublas::sum(MatrixRow(st_trans, i));

			// estimate observations
			for (size_t j = 0; j < num_obs; ++j)
			{
				float newparam = 0.0f;

				for (size_t d = 0; d < obs_seqs.size(); ++d)
				{
					auto& C = Cs[d];
					auto& Ds = Dss[d];
					const auto& obs_seq = obs_seqs[d];

					const float csum_1_to_T = ublas::sum(
						MatrixCol(C, i)) - C(0, i);
					ATP_STATS_ASSERT(csum_1_to_T != 0.0f);

					float csum_jth_obs = 0.0f;
					for (size_t t = 0; t < obs_seq.size(); ++t)
					{
						if (obs_seq[t] == j)
							csum_jth_obs += C(t, i);
						else
							// always make sure to do smoothing
							csum_jth_obs += smoothing;
					}

					newparam += csum_jth_obs / csum_1_to_T;
				}

				// average over all data points, and apply decay:
				st_obs(i, j) = (1.0f - decay) * st_obs(i, j) +
					decay * newparam / (float)obs_seqs.size();
			}

			// normalise, just in case
			MatrixRow(st_obs, i) /= ublas::sum(MatrixRow(st_obs, i));
		}
	}
}


}  // namespace stats
}  // namespace atp


