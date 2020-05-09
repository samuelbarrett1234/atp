SELECT stmt, num_attempts, num_proofs, time_agg, max_mem_agg, num_exp_agg,

-- compute the number of times this theorem could've been / was used in a proof
IFNULL((SELECT SUM(cnt) FROM theorem_usage WHERE id=used_thm_id), 0) AS num_uses,

  -- convert date from julianday to something readable
DATE(thm_date) as date_entered

FROM theorems JOIN

(
	-- get aggregate information about each theorem's proof attempts
	(SELECT thm_id, SUM(time_cost) AS time_agg, MAX(max_mem) AS max_mem_agg,
		SUM(num_expansions) AS num_exp_agg, COUNT(thm_id) AS num_attempts
		FROM proof_attempts GROUP BY thm_id)
		
	NATURAL JOIN
	
	-- get whether or not each statement has been proven (num proofs will
	-- always be 0 or 1)
	(SELECT id AS thm_id,
		(CASE thm_id IS NULL WHEN TRUE THEN 0 ELSE 1 END)
		AS num_proofs FROM theorems LEFT OUTER JOIN proofs ON id=thm_id)
)

ON id=thm_id

ORDER BY num_proofs DESC, num_uses DESC, num_attempts DESC
;
