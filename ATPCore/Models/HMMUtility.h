#pragma once


/**
\file

\author Samuel Barrett

\brief Contains Hidden Markov Model training functions.

*/


#include <vector>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
{
namespace hmm
{


/**
\brief Perform the "forward algorithm" for hidden Markov models.

\returns A matrix whose t^th row and i^th column represents the
	probability that the chain was in state i at time t AND we
	observed what we did from timesteps 0..t (where t and i are
	zero-indexed).

\param initial_state The initial state distribution

\param st_trans The state transition matrix

\param st_obs The state observation matrix

\param obs_seq A single observed sequence

\pre st_trans.size1() == initial_state.size()

\pre st_trans.size1() == st_trans.size2()

\pre st_obs.size2() == st_trans.size1()

*/
ATP_CORE_API boost::numeric::ublas::matrix<float> forward(
	boost::numeric::ublas::vector<float>& initial_state,
	boost::numeric::ublas::matrix<float>& st_trans,
	boost::numeric::ublas::matrix<float>& st_obs,
	const std::vector<size_t>& obs_seq);

/**
\brief Perform the "backward algorithm" for hidden Markov models.

\returns A matrix whose t^th row and i^th column represents the
	probability that the chain would encounter observations from
	t+1 onwards, given that it was in state i at time t.

\param st_trans The state transition matrix

\param st_obs The state observation matrix

\param obs_seq A single observed sequence

\pre st_trans.size1() == st_trans.size2()

\pre st_obs.size2() == st_trans.size1()

*/
ATP_CORE_API boost::numeric::ublas::matrix<float> backward(
	boost::numeric::ublas::matrix<float>& st_trans,
	boost::numeric::ublas::matrix<float>& st_obs,
	const std::vector<size_t>& obs_seq);

/**
\brief Perform N iterations of the Baum-Welch training update to
	the internal model parameters.

\param initial_state The initial state distribution

\param st_trans The state transition matrix

\param st_obs The state observation matrix

\param obs_seqs The observed sequences (this is the data)

\param num_epochs The number of training epochs to repeat (this will
	be equal to the number of times each datapoint is trained on).

\param smoothing The Laplace smoothing amount

\param decay Multiplier for the update to the model parameters (0
	will mean that this update has no effect, 1 means we will forget
	the last set of parameters, 0.5 means equal average the two, etc)

\pre st_trans.size1() == initial_state.size()

\pre st_trans.size1() == st_trans.size2()

\pre st_obs.size2() == st_trans.size1()

\pre smoothing >= 0.0f
*/
ATP_CORE_API void baum_welch(
	boost::numeric::ublas::vector<float>& initial_state,
	boost::numeric::ublas::matrix<float>& st_trans,
	boost::numeric::ublas::matrix<float>& st_obs,
	const std::vector<std::vector<size_t>>& obs_seqs,
	size_t num_epochs, float smoothing, float decay);


}  // namespace hmm
}  // namespace core
}  // namespace atp


