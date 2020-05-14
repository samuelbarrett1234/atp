SELECT stmt, num_attempts, time_agg, max_mem_agg, num_exp_agg,

-- compute the number of times this theorem could've been / was used in a proof
IFNULL((SELECT SUM(cnt) FROM theorem_usage WHERE id=used_thm_id), 0) AS num_uses,

  -- convert date from julianday to something readable
DATE(thm_date) AS date_entered,

DATE(proof_date) AS date_proven

FROM theorems JOIN

(
	-- get aggregate information about each theorem's proof attempts
	(SELECT thm_id, SUM(time_cost) AS time_agg, MAX(max_mem) AS max_mem_agg,
		SUM(num_expansions) AS num_exp_agg, COUNT(thm_id) AS num_attempts
		FROM proof_attempts GROUP BY thm_id)
		
	NATURAL JOIN proofs
)

ON id=thm_id

ORDER BY num_uses DESC, num_attempts DESC
;
