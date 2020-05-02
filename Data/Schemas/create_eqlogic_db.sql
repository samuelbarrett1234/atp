CREATE TABLE IF NOT EXISTS model_contexts (
	ctx_id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL, filename TEXT);


INSERT INTO model_contexts (name, filename) VALUES (
	"group-theory", "../Data/Definitions/group_theory.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"prop-logic", "../Data/Definitions/prop_logic.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"reg-expr", "../Data/Definitions/reg_expr.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"ring-arithmetic", "../Data/Definitions/ring_arithmetic.json");


CREATE TABLE IF NOT EXISTS theorems (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	stmt TEXT NOT NULL UNIQUE,  /* target statement as text */
	ctx INTEGER NOT NULL,  /* the model context */
	FOREIGN KEY (ctx) REFERENCES model_contexts(ctx_id));


/*
a proof attempt is successful iff there exists an entry in the proof
table which references this attempt.
*/
CREATE TABLE IF NOT EXISTS proof_attempts (
	attempt_id INTEGER PRIMARY KEY AUTOINCREMENT,
	thm_id INTEGER UNIQUE NOT NULL,	
	time_cost REAL NOT NULL,
	max_mem INTEGER NOT NULL,
	num_expansions INTEGER NOT NULL,
	FOREIGN KEY(thm_id) REFERENCES theorems(id)
	);


CREATE TABLE IF NOT EXISTS proofs (
	attempt_id INTEGER UNIQUE NOT NULL,
	proof TEXT,  /* human readable proof as string */
	FOREIGN KEY (attempt_id) REFERENCES proof_attempts(id));


CREATE INDEX IF NOT EXISTS thm_by_id ON theorems(id);
CREATE INDEX IF NOT EXISTS thm_stmt ON theorems(stmt);
CREATE INDEX IF NOT EXISTS pf_attempts ON proof_attempts(attempt_id);
CREATE INDEX IF NOT EXISTS pfs ON proofs(attempt_id);

