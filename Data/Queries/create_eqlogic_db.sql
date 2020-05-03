PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS model_contexts (
	ctx_id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL, filename TEXT);


INSERT INTO model_contexts (name, filename) VALUES (
	"group-theory", "../Data/Definitions/group_theory.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"boolean-algebra", "../Data/Definitions/boolean_algebra.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"reg-expr", "../Data/Definitions/reg_expr.json");
INSERT INTO model_contexts (name, filename) VALUES (
	"ring-arithmetic", "../Data/Definitions/ring_arithmetic.json");


CREATE TABLE IF NOT EXISTS search_settings (
	ss_id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL, filename TEXT);


INSERT INTO search_settings (name, filename) VALUES (
	"ids-uninformed-limited",
	"../Data/Search/ids_uninformed_limited.json");
INSERT INTO search_settings (name, filename) VALUES (
	"ids-uninformed-extended",
	"../Data/Search/ids_uninformed_extended.json");
INSERT INTO search_settings (name, filename) VALUES (
	"ids-informed-limited",
	"../Data/Search/ids_informed_limited.json");
INSERT INTO search_settings (name, filename) VALUES (
	"ids-informed-extended",
	"../Data/Search/ids_informed_extended.json");


CREATE TABLE IF NOT EXISTS theorems (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	stmt TEXT NOT NULL UNIQUE,  /* target statement as text */
	ctx INTEGER NOT NULL,  /* the model context */
	FOREIGN KEY (ctx) REFERENCES model_contexts(ctx_id));


/*
a proof attempt is successful iff there exists an entry in the proof
table which references this attempt.
proof attempts also reference the search settings they were executed
with, if necessary.
*/
CREATE TABLE IF NOT EXISTS proof_attempts (
	thm_id INTEGER NOT NULL,
	ss_id INTEGER,  /* may be null, if using alternative settings */
	time_cost REAL NOT NULL,
	max_mem UNSIGNED INTEGER NOT NULL,
	num_expansions UNSIGNED INTEGER NOT NULL,
	FOREIGN KEY(thm_id) REFERENCES theorems(id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	FOREIGN KEY(ss_id) REFERENCES search_settings(ss_id));


/*
A theorem is proven true iff there exists a proof of it in this
table.
We also store the proofs as text, however we suspect that not to
be very useful!
Whenever there is a proof of a theorem, there is also a proof
attempt.
*/
CREATE TABLE IF NOT EXISTS proofs (
	thm_id INTEGER UNIQUE NOT NULL,
	proof TEXT,  /* human readable proof as string */
	FOREIGN KEY (thm_id) REFERENCES theorems(id)
	ON DELETE CASCADE ON UPDATE CASCADE);
	
	
/*
External information about theorems which need to be proven. Any
theorem which does not have a corresponding task entry can be
thought of as a "lemma" - not that important to the outside worlld
but may be important to proving one of the tasks in this table.
*/
CREATE TABLE IF NOT EXISTS tasks (
	thm_id INTEGER UNIQUE NOT NULL,
	deadline REAL,  /* number of days from noon in Greenwich on
	                   November 24, 4714 B.C up to deadline */
	priority REAL,
	FOREIGN KEY (thm_id) REFERENCES theorems(id)
	ON DELETE CASCADE ON UPDATE CASCADE
	);


CREATE INDEX IF NOT EXISTS thm_by_id ON theorems(id);
CREATE INDEX IF NOT EXISTS thm_stmt ON theorems(stmt);
CREATE INDEX IF NOT EXISTS attempt_thm_ids ON proof_attempts(thm_id);
CREATE INDEX IF NOT EXISTS proof_thm_ids ON proofs(thm_id);
CREATE INDEX IF NOT EXISTS task_thm_ids ON tasks(thm_id);
