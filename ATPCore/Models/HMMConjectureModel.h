#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a Hidden Markov model implementation for the
	automated conjecturer.

*/


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
	latter does not give symmetry.

\invariant The model always holds a distribution of hidden states,
	and always has a current observation (which was generated from
	the current state distribution).
*/
class ATP_CORE_API HMMConjectureModel
{
public:
	HMMConjectureModel(logic::ModelContextPtr p_ctx);

	/**
	\brief Reset state distribution to the default (typically just a
		uniform distribution).
	*/
	void reset_state();

	/**
	\brief Advance the state one step and generate a new observation.
	*/
	void advance();

	/*
	\brief Either return the current symbol ID, or the free variable
		ID.
	*/
	size_t current_observation() const;

	/**
	\returns True iff the current observation is a free variable.
	*/
	bool current_observation_is_free_var() const;

	/**
	\returns The arity of the current symbol ID (which will be 0 if
		current_observation_is_free_var() returns true).
	*/
	size_t current_observation_arity() const;
};


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
	*/
	std::unique_ptr<HMMConjectureModel> build() const;

	void set_num_hidden_states(size_t n);
	void add_state_transition(size_t pre, size_t post, float p);
	void add_symbol_observation(
		size_t state, size_t symb_id, float p);
	void set_free_geometric_q(float q);
};


}  // namespace core
}  // namespace atp


