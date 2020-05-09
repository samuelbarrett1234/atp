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
#include "HMMStmtToObs.h"


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
	/**
	\param rand_seed A seed for the random generator for the
		conjectures.

	\param smoothing The Laplace smoothing amount for estimating the
		probabilities during training.

	\param free_q The q parameter for the geometric distribution
		which generates the free variable IDs.
	*/
	HMMConjectureModel(logic::ModelContextPtr p_ctx,
		size_t num_states, float free_q,
		boost::numeric::ublas::matrix<float> st_trans,
		boost::numeric::ublas::matrix<float> st_obs,
		std::vector<size_t> symbs,
		size_t rand_seed, float smoothing);

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
	void advance();

	/**
	\brief Train on each statement N times

	\pre The statements in p_stmts are associated with the model
		context given in the constructor.

	\warning Might interfere with any existing conjecturing processes
		going on, as it modifies the matrices.
	*/
	void train(const logic::StatementArrayPtr& p_stmts, size_t N);

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

	inline const boost::numeric::ublas::matrix<float>& get_st_trans() const
	{
		return m_st_trans;
	}
	inline const boost::numeric::ublas::matrix<float>& get_st_obs() const
	{
		return m_st_obs;
	}
	inline const std::vector<size_t>& get_symbols() const
	{
		return m_symbs;
	}
	inline float free_q() const
	{
		return m_free_q;
	}
	inline size_t num_states() const
	{
		return m_num_hidden_states;
	}
	inline float st_trans_prob(size_t from, size_t to) const
	{
		ATP_CORE_PRECOND(from < m_num_hidden_states);
		ATP_CORE_PRECOND(to < m_num_hidden_states);
		ATP_CORE_ASSERT(m_st_trans.size1() ==
			m_num_hidden_states);
		ATP_CORE_ASSERT(m_st_trans.size2() ==
			m_num_hidden_states);

		return m_st_trans(from, to);
	}
	inline float obs_prob(size_t state, size_t symb_id) const
	{
		ATP_CORE_PRECOND(state < m_num_hidden_states);

		const auto iter = std::find(m_symbs.begin(), m_symbs.end(),
			symb_id);
		ATP_CORE_PRECOND(iter != m_symbs.end());

		const size_t idx = std::distance(m_symbs.begin(), iter);
		ATP_CORE_ASSERT(idx < m_st_obs.size2());

		ATP_CORE_ASSERT(m_st_obs.size1() == m_num_hidden_states);
		ATP_CORE_ASSERT(m_st_obs.size2() == m_symbs.size());

		return m_st_obs(state, idx);
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
		m_st_obs_partial_sums, m_st_obs;

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

	// for training
	HMMStmtToObs m_stmt_to_obs;
	const float m_smoothing;
};


}  // namespace core
}  // namespace atp


