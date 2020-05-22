PRAGMA foreign_keys = ON;


CREATE TABLE IF NOT EXISTS model_contexts (
	ctx_id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL, filename TEXT);


INSERT OR IGNORE INTO model_contexts (name, filename) VALUES (
	"group-theory", "../Data/Definitions/group_theory.json");
INSERT OR IGNORE INTO model_contexts (name, filename) VALUES (
	"boolean-algebra", "../Data/Definitions/boolean_algebra.json");
INSERT OR IGNORE INTO model_contexts (name, filename) VALUES (
	"reg-expr", "../Data/Definitions/reg_expr.json");
INSERT OR IGNORE INTO model_contexts (name, filename) VALUES (
	"ring-arithmetic", "../Data/Definitions/ring_arithmetic.json");
INSERT OR IGNORE INTO model_contexts (name, filename) VALUES (
	"calculus-algebra", "../Data/Definitions/calculus_algebra.json");


CREATE TABLE IF NOT EXISTS search_settings (
	ss_id INTEGER PRIMARY KEY AUTOINCREMENT,
	name TEXT UNIQUE NOT NULL, filename TEXT,
	
	-- whether or not these settings can be used in serve mode (where
	-- everything is automated). you probably want to set this to 0
	-- for extended settings, to stop the server from selecting the
	-- more computationally expensive settings.
	serve INTEGER NOT NULL,
	CHECK(serve IN (0, 1)));


INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-uninformed-limited",
	"../Data/Search/ids_uninformed_limited.json", 1);
INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-uninformed-extended",
	"../Data/Search/ids_uninformed_extended.json", 0);
INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-uninformed-large",
	"../Data/Search/ids_uninformed_large.json", 0);
INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-informed-tiny",
	"../Data/Search/ids_informed_tiny.json", 1);
INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-informed-limited",
	"../Data/Search/ids_informed_limited.json", 1);
INSERT OR IGNORE INTO search_settings (name, filename, serve) VALUES (
	"ids-informed-extended",
	"../Data/Search/ids_informed_extended.json", 0);


CREATE TABLE IF NOT EXISTS theorems (
	thm_id INTEGER PRIMARY KEY AUTOINCREMENT,
	stmt TEXT NOT NULL,	/* target statement as text */
	ctx_id INTEGER NOT NULL,	/* the model context */
	
	/*
	Number of days from noon in Greenwich on
	November 24, 4714 B.C up to the time where this
	theorem was entered into the database.
	*/
	thm_date REAL DEFAULT (julianday('now')),
	
	/*
	Users assign priorities to tasks they set, however these imply
	priorities for theorems in the database (because if A=>B and
	B has priority x, then A must have priority >=x)
	*/
	induced_priority REAL NOT NULL,
	
	FOREIGN KEY (ctx_id) REFERENCES model_contexts(ctx_id),
	UNIQUE(stmt, ctx_id)	/* can have duplicate stmts across different contexts */);


/*
a proof attempt is successful iff there exists an entry in the proof
table which references this attempt.
proof attempts also reference the search settings they were executed
with, if necessary.
*/
CREATE TABLE IF NOT EXISTS proof_attempts (
	thm_id INTEGER NOT NULL,
	
	/*
	Whether this attempt was successful (0 or 1).
	If this is 1, it is the only proof attempt for this theorem with
	success=1, and there will exist a corresponding entry in the
	`proof` table.
	*/
	success INTEGER NOT NULL,
	
	/* may be null, if using alternative settings */
	ss_id INTEGER,
	time_cost REAL NOT NULL,
	max_mem UNSIGNED INTEGER,
	num_expansions UNSIGNED INTEGER,
	
	/*
	Number of days from noon in Greenwich on
	November 24, 4714 B.C up to the time where this
	proof attempt was made.
	*/
	attempt_date REAL DEFAULT (julianday('now')),
	
	/*
	Information about who ran the proof
	*/
	app_version TEXT,  -- e.g. "0.2"
	app_build TEXT,  -- e.g. "Debugx64"
	machine_name TEXT,  -- e.g. "Samuel's PC"
	
	CHECK (success IN (0, 1)),
	FOREIGN KEY(thm_id) REFERENCES theorems(thm_id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	FOREIGN KEY(ss_id) REFERENCES search_settings(ss_id));


/*
A theorem is proven true iff there exists a proof of it in this
table.
We also store the proofs as text, however we suspect that not to
be very useful! Do not rely on the structure of the proofs too much.
Whenever there is a proof of a theorem, there is also a proof
attempt, UNLESS it is an AXIOM!
Note that we store which theorems were used in the proof in a
separate table.
*/
CREATE TABLE IF NOT EXISTS proofs (
	thm_id INTEGER UNIQUE NOT NULL,
	
	/*
	human readable proof as string, line-separated, can be null
	*/
	proof TEXT,
	
	/*
	1 if this is an axiom, in which case the proof will be null/empty,
	or 0 if this is an actual theorem.
	*/
	is_axiom INTEGER NOT NULL DEFAULT 0,
	
	/*
	Number of days from noon in Greenwich on
	November 24, 4714 B.C up to the time where this
	proof was entered into the database (when the
	theorem was proven).
	*/
	proof_date REAL DEFAULT (julianday('now')),
	
	CHECK(is_axiom IN (0, 1)),
	FOREIGN KEY (thm_id) REFERENCES theorems(thm_id)
	ON DELETE CASCADE ON UPDATE CASCADE);


/*
Every entry in this table represents the fact that the `used_thm_id`
was used (or could've been used) in the proof of `target_thm_id`.
This is a very "approximate" notion, and this table is only intended
to be used for computing "usefulness" heuristics of each theorem.

DO NOT rely on this as a way of checking proofs, this is not guaranteed
to truly represent the proofs given in the other table. Instead, find a
way to deserialise the proof text.
*/
CREATE TABLE IF NOT EXISTS theorem_usage (
	target_thm_id INTEGER NOT NULL,
	used_thm_id INTEGER NOT NULL,
	cnt UNSIGNED INTEGER NOT NULL,	-- the count (number of usages)
	
	CHECK(cnt > 0),
	CHECK(target_thm_id != used_thm_id),	-- avoid circularity!
	
	FOREIGN KEY (target_thm_id) REFERENCES proofs(thm_id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	FOREIGN KEY (used_thm_id) REFERENCES proofs(thm_id)
	ON DELETE CASCADE ON UPDATE CASCADE,
	UNIQUE(target_thm_id, used_thm_id)
	);


/*
External information about theorems which need to be proven. Any
theorem which does not have a corresponding task entry can be
thought of as a "lemma" - not that important to the outside worlld
but may be important to proving one of the tasks in this table.
*/
CREATE TABLE IF NOT EXISTS tasks (
	thm_id INTEGER UNIQUE NOT NULL,
	deadline REAL,	/* number of days from noon in Greenwich on
										 November 24, 4714 B.C up to deadline */
	priority REAL,
	FOREIGN KEY (thm_id) REFERENCES theorems(thm_id)
	ON DELETE CASCADE ON UPDATE CASCADE
	);


CREATE INDEX IF NOT EXISTS thm_by_id ON theorems(thm_id);
CREATE INDEX IF NOT EXISTS thm_stmt ON theorems(stmt);
CREATE INDEX IF NOT EXISTS attempt_thm_ids ON proof_attempts(thm_id);
CREATE INDEX IF NOT EXISTS proof_thm_ids ON proofs(thm_id);
CREATE INDEX IF NOT EXISTS task_thm_ids ON tasks(thm_id);
CREATE INDEX IF NOT EXISTS thm_usage ON theorem_usage(used_thm_id);	-- not target_thm_id

