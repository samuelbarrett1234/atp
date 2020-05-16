<!DOCTYPE html>
<html lang="en">
<!--#include file="data.php" -->

<!--
Template obtained from https://www.w3schools.com/w3css/w3css_templates.asp
-->

<body>
<!--#include file="navbar.php" -->
<!--#include file="header.php" -->

<!-- First Grid -->
<div class="w3-row-padding w3-padding-64 w3-container">
	<div class="w3-content">
		<h1 id='pf_start'>Tasks</h1>
		<h5 class="w3-padding-32">Below is a catalogue of all of the prover's user-submitted tasks and their statuses</h5>
		<?php
		// connect to database
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// offset (which defaults to zero)
		$off = 0;
		if (array_key_exists('off', $_GET) && $_GET['off'] != null)
		{
			$off = (int)$_GET['off'];
		}
		
		// big query:
		$prep = $db->prepare("SELECT stmt, name AS ctx_name, is_proven, num_attempts, priority,

		  -- convert date from julianday to something readable
		DATE(thm_date) AS date_entered,

		DATE(proof_date) AS date_proven,

		DATE(deadline) AS date_deadline

		FROM theorems JOIN

		(
		SELECT thm_id, IFNULL(num_attempts, 0) as num_attempts,
		IFNULL(is_proven, 'No') as is_proven, proof_date, priority, deadline
		
		FROM tasks NATURAL LEFT OUTER JOIN
		
		(
			SELECT thm_id, num_attempts, proof_date,
			(CASE WHEN proof IS NULL THEN 'No' ELSE 'Yes' END) as is_proven
			
			FROM
			
			-- get aggregate information about each theorem's proof attempts
			(SELECT thm_id, COUNT(thm_id) AS num_attempts
				FROM proof_attempts GROUP BY thm_id)
			
			NATURAL LEFT OUTER JOIN proofs
		))

		ON id=thm_id
		
		JOIN model_contexts ON ctx = ctx_id

		ORDER BY date_deadline DESC
		LIMIT 25 OFFSET :off
		");
		
		$prep->bindValue(':off', $off, SQLITE3_INTEGER);
		$query = $prep->execute();
		
		// create table
		echo "<br><table class=\"w3-text-grey w3-table-all\">\n";
		
		// create headers
		echo "<tr><th>Statement</th><th>Context</th><th>Proven</th><th>Number of proof attempts</th><th>Date entered</th><th>Date proven</th><th>Deadline</th><th>Priority</th></tr>\n";
		
		// enumerate results
		while ($row = $query->fetchArray())
		{
			// output row of table
			echo "<tr><td><a href='examine_thm.php?stmt={$row['stmt']}&ctx={$row['ctx_name']}'>{$row['stmt']}</a></td><td>{$row['ctx_name']}</td><td>{$row['is_proven']}</td><td>{$row['num_attempts']}</td><td>{$row['date_entered']}</td><td>{$row['date_proven']}</td><td>{$row['date_deadline']}</td><td>{$row['priority']}</td></tr>\n";
		}
		echo "</table>\n";
		
		// the "more..." link
		$off += 25;
		echo "<br><br><a href='view_pf_data.php?off={$off}#pf_start' class='w3-text-grey'>More...</a>\n";
		?>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
