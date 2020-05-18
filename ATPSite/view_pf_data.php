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
		<h1 id='pf_start'>Proof Data</h1>
		<?php
		// connect to database
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// offset (which defaults to zero)
		$off = 0;
		if (array_key_exists('off', $_GET) && $_GET['off'] != null)
		{
			$off = (int)$_GET['off'];
		}
		
		// context
		$ctx_name = "group-theory";
		if (array_key_exists('ctx', $_GET) && $_GET['ctx'] != null)
		{
			$ctx_name = $_GET['ctx'];
		}
		
		echo "<h5 class=\"w3-padding-32\">Below is a catalogue of some proven theorems in {$ctx_name} and information about them</h5>\n";
		
		// big query:
		$prep = $db->prepare("SELECT stmt, num_attempts, time_agg, max_mem_agg, num_exp_agg,

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
		
		JOIN model_contexts ON ctx = ctx_id
		WHERE name = :ctx_name
		AND is_axiom = 0  -- no axioms please

		ORDER BY num_uses DESC, num_attempts DESC
		LIMIT 25 OFFSET :off
		");
		
		$prep->bindValue(':off', $off, SQLITE3_INTEGER);
		$prep->bindValue(':ctx_name', $ctx_name, SQLITE3_TEXT);
		$query = $prep->execute();
		
		// create table
		echo "<br><table class=\"w3-text-grey w3-table-all\">\n";
		
		// create headers
		echo "<tr><th>Statement</th><th>Number of attempts</th><th>Time cost (s)</th><th>Max memory usage</th><th>Num node expansions</th><th>Num usages</th><th>Date entered</th><th>Date proven</th></tr>\n";
		
		// enumerate results
		while ($row = $query->fetchArray())
		{
			// output row of table
			echo "<tr><td><a href='examine_thm.php?stmt={$row['stmt']}&ctx={$ctx_name}'>{$row['stmt']}</a></td><td>{$row['num_attempts']}</td><td>{$row['time_agg']}</td><td>{$row['max_mem_agg']}</td><td>{$row['num_exp_agg']}</td><td>{$row['num_uses']}</td><td>{$row['date_entered']}</td><td>{$row['date_proven']}</td></tr>\n";
		}
		echo "</table>\n";
		
		// the "next..." link
		$off += 25;
		echo "<br><a href='view_pf_data.php?off={$off}&ctx={$ctx_name}#pf_start' class='w3-text-grey'>Next...</a>\n";
		
		// form for changing contexts
		echo "<br><br><hr><br>\n";
		echo "<form action=\"view_pf_data.php\" method=get>\n";
		echo "Change context:\n";
		echo "<select name=\"ctx\" class=\"w3-select\">\n";
		
		$query = $db->query("SELECT name FROM model_contexts");
		
		// enumerate the model contexts to select from
		while ($row = $query->fetchArray())
		{
			echo "<option value=\"{$row['name']}\" class=\"w3-input w3-option\"";
			if ($ctx_name == $row['name'])
			{
				echo " selected=\"selected\"";
			}
			echo ">{$row['name']}</option>\n";
		}
		?>
		</select>
		<br><br>
		<input type=submit value="Go" class="w3-input w3-button">
		</form>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
