SELECT stmt, time_agg, max_mem_agg, num_exp_agg, num_proofs
FROM theorems JOIN

(
	-- get aggregate information about each theorem's proof attempts
	(SELECT thm_id, SUM(time_cost) as time_agg, MAX(max_mem) as max_mem_agg,
		SUM(num_expansions) as num_exp_agg
		FROM proof_attempts GROUP BY thm_id)
		
	NATURAL JOIN
	
	-- get whether or not each statement has been proven (num proofs will
	-- always be 0 or 1)
	(SELECT id as thm_id, (CASE thm_id IS NULL WHEN TRUE THEN 0 ELSE 1 END) as num_proofs FROM theorems LEFT OUTER JOIN proofs ON id=thm_id)
)

ON id=thm_id;
