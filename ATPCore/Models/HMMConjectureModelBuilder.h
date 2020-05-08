#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a class for building a HMMConjectureModel (which is
	useful loading one from the database).

*/


#include <map>
#include <boost/optional.hpp>
#include <ATPLogic.h>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
{


class HMMConjectureModel;  // forward declaration


/**
\brief A class for building a HMMConjectureModel out of tid-bits from
	the database.

\details This is to ensure that, once a model is obtained, it is
	correct (invariants and preconditions satisfied).
*/
class ATP_CORE_API HMMConjectureModelBuilder
{
public:
	HMMConjectureModelBuilder(logic::ModelContextPtr p_ctx);

	/**
	\brief Determine if the parameters that have been set so far are
		valid.
	*/
	bool can_build() const;

	/**
	\brief Create a new HMMConjectureModel given the parameters

	\pre can_build()

	\returns A new HMMConjectureModel object
	*/
	std::unique_ptr<HMMConjectureModel> build() const;

	/**
	\brief Restore this object to the state it was when it was
		constructed.
	*/
	void reset();

	/**
	\brief Set the number of hidden states. The states will then be
		thought of as labelled 0..n-1 inclusive.
	*/
	void set_num_hidden_states(size_t n)
	{
		m_num_states = n;

		m_valid = m_valid && *m_num_states > 0;
	}

	/**
	\brief Set the fact that the system will transition from state
		`pre` to state `post` with probability p
	*/
	void add_state_transition(size_t pre, size_t post, float p)
	{
		m_state_trans[std::make_pair(pre, post)] = p;

		m_valid = m_valid && p >= 0.0f && p <= 1.0f;
	}

	/**
	\brief Set the fact that, when in state `state`, we will observe
		the symbol `symbol` with probability p.

	\note This only applies to functions and constants, not to free
		variables.
	*/
	void add_symbol_observation(
		size_t state, std::string symbol, float p);

	/**
	\brief Set the parameter q=1-p for generating the free variable
		IDs, where **p** (not q) is the parameter for the geometric
		distribution.
	*/
	inline void set_free_geometric_q(float q)
	{
		m_q = q;

		m_valid = m_valid && m_q >= 0.0f && m_q <= 1.0f;
	}

	inline void set_random_seed(size_t rand_seed)
	{
		m_rand_seed = rand_seed;
	}

	inline void set_smoothing(float smoothing)
	{
		m_smoothing = smoothing;
	}

private:
	/**
	\brief Perform validity checking on state transitions

	\pre m_num_states.has_value() && m_valid

	\returns True iff m_state_trans represents a correct state
		transition matrix (and of the correct size)
	*/
	bool check_state_trans() const;

	/**
	\brief Perform validity checking on observation matrix

	\pre m_num_states.has_value() && m_valid

	\returns True iff m_symb_obs represents a correct observation
		matrix (and of the correct size), of course excluding free
		variables.
	*/
	bool check_obs() const;

private:
	logic::ModelContextPtr m_ctx;

	bool m_valid;  // check validity as we go along
	// invariant: m_valid is true iff all the probabilities contained
	// herein are between 0 and 1, and all symbol IDs are valid,
	// and that we have a nonzero number of hidden states.

	float m_smoothing = 1.0e-6f;

	boost::optional<float> m_q;
	boost::optional<size_t> m_num_states, m_rand_seed;
	std::map<std::pair<size_t, size_t>, float> m_state_trans,
		m_symb_obs;
};


}  // namespace core
}  // namespace atp


