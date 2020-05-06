#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a Hidden Markov model implementation for the
	automated conjecturer.

*/


#include <random>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <ATPLogic.h>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
{


/**
\brief A hidden Markov model implementation, which has an arbitrary
	number of hidden states, and some fixed structure on the
	observation states.

\details The observed states are any of the following: a function or
	constant symbol, or a free variable ID. Of course, the symbols
	are relatively straightforward, but with free variables we need
	to generate an ID. To do this, we just draw from a geometric
	distribution! Furthermore, one might ask how we string together
	the observations. We can do one of two methods. (i) propagate
	the state probabilities downwards without letting them interact
	and propagate back upwards again, or (ii) we can make the states
	propagate in pre-order traversal of the array. The former does
	not allow information-sharing between disjoint subtrees, and the
	latter does not give symmetry. You are free to decide which you
	will use as you are generating the results.

\invariant The model always holds a distribution of hidden states,
	and always has a current observation (which was generated from
	the current state distribution).
*/
class ATP_CORE_API HMMConjectureModel
{
public:
	HMMConjectureModel(logic::ModelContextPtr p_ctx,
		size_t num_states, float free_q,
		boost::numeric::ublas::matrix<float> st_trans,
		boost::numeric::ublas::matrix<float> st_obs,
		std::vector<size_t> symbs,
		size_t rand_seed);

	/**
	\brief Reset state distribution to the default (typically just a
		uniform distribution).
	*/
	void reset_state()
	{
		// initial state always uniform
		m_state = boost::numeric::ublas::scalar_vector<float>(
			m_num_hidden_states,
			1.0f / (float)m_num_hidden_states);

		generate_observation();
	}

	/**
	\brief Advance the state one step and generate a new observation.
	*/
	inline void advance()
	{
		m_state = boost::numeric::ublas::prod(m_st_trans, m_state);
		generate_observation();
	}

	/*
	\brief Either return the current symbol ID, or the free variable
		ID.
	*/
	inline size_t current_observation() const
	{
		return m_cur_obs;
	}

	/**
	\returns True iff the current observation is a free variable.
	*/
	inline bool current_observation_is_free_var() const
	{
		return m_cur_obs_is_free;
	}

	/**
	\returns The arity of the current symbol ID (which will be 0 if
		current_observation_is_free_var() returns true).
	*/
	inline size_t current_observation_arity() const
	{
		return m_cur_obs_arity;
	}

private:
	void generate_observation();

private:
	logic::ModelContextPtr m_ctx;

	const size_t m_num_hidden_states;
	const float m_free_q;
	std::vector<size_t> m_symbs;  // symbol IDs in some order

	boost::numeric::ublas::vector<float> m_state;
	boost::numeric::ublas::matrix<float> m_st_trans,
		m_st_obs_partial_sums;

	// Note on state observation matrix:
	// This is the state observation matrix. Each row represents a
	// symbol observation (not a free variable observation). The
	// symbol represented by row i is m_symbs[i]
	// However, the state observation matrix is never used directly
	// and instead we only need the probabilities that we observe
	// a symbol occurring in the range m_symbs[0..i] inclusive.

	// info about current observation:
	size_t m_cur_obs, m_cur_obs_arity;
	bool m_cur_obs_is_free;

	// randomness:
	std::mt19937 m_rand_device;
	std::uniform_real_distribution<float> m_unif01;  // uniform [0,1]
};


}  // namespace core
}  // namespace atp


