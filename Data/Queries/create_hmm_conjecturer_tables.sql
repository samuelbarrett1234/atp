CREATE TABLE IF NOT EXISTS hmm_conjecturer_models (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	ctx_id INTEGER NOT NULL,
	
	/*
	The number of hidden states of the HMM.
	*/
	num_states INTEGER NOT NULL,
	
	/*
	The probability associated with the free variable ID
	geometric distribution. This is "q" (not "p") and the
	geometric distribution parameter would be p=1-q.
	*/
	free_q REAL NOT NULL,
	
	CHECK(free_q >= 0.0 AND free_q <= 1.0),
	CHECK(num_states > 0),
	CHECK(id >= 0)
);

CREATE TABLE IF NOT EXISTS hmm_conjecturer_state_transitions (
	id INTEGER NOT NULL,
	
	-- probability of transitioning from pre_state to post_state
	pre_state INTEGER NOT NULL,
	post_state INTEGER NOT NULL,
	prob REAL NOT NULL,
	
	/*
	TODO: find a way of checking that the pre_state and the
	post_state are in the range 0..num_states-1 where num_states
	is the corresponding number of states of the Markov chain
	(in hmm_conjecturer_models).
	*/
	CHECK(prob >= 0.0 AND prob <= 1.0),
	
	FOREIGN KEY(id) REFERENCES hmm_conjecturer_models(id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	
	UNIQUE(id, pre_state, post_state)
);

CREATE TABLE IF NOT EXISTS hmm_conjecturer_symbol_observations (
	id INTEGER NOT NULL,
	
	hidden_state INTEGER NOT NULL,
	symbol TEXT NOT NULL,
	prob REAL NOT NULL,
	
	/*
	TODO: find a way of checking that the pre_state and the
	post_state are in the range 0..num_states-1 where num_states
	is the corresponding number of states of the Markov chain
	(in hmm_conjecturer_models).
	*/
	CHECK(prob >= 0.0 AND prob <= 1.0),
	
	FOREIGN KEY(id) REFERENCES hmm_conjecturer_models(id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	
	UNIQUE(id, hidden_state, symbol)
);

