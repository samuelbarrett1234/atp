/*
create an arbitrary HMM for group theory with 4 hidden states
OVERWRITES EXISTING ONE IF EXISTS
*/


INSERT OR REPLACE INTO hmm_conjecturer_models (id, ctx_id, num_states, free_q) VALUES (1, 1, 4, 0.25);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 0, 0, 0.1);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 0, 1, 0.2);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 0, 2, 0.3);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 0, 3, 0.4);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 1, 0, 0.3);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 1, 1, 0.3);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 1, 2, 0.3);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 1, 3, 0.1);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 2, 0, 0.8);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 2, 1, 0.05);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 2, 2, 0.05);

INSERT OR REPLACE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 2, 3, 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 3, 0, 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 3, 1, 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 3, 2, 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_state_transitions (id, pre_state, post_state, prob) VALUES (1, 3, 3, 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 0, 'e', 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 0, 'i', 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 0, '*', 0.25);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 1, 'e', 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 1, 'i', 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 1, '*', 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 2, 'e', 0.6);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 2, 'i', 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 2, '*', 0.1);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 3, 'e', 0.05);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 3, 'i', 0.6);

INSERT OR IGNORE INTO hmm_conjecturer_symbol_observations (id, hidden_state, symbol, prob) VALUES (1, 3, '*', 0.3);

