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
		<div class="w3-twothird">
		<h1>Statement Info</h1>
		<?php
		echo "<h5 class='w3-padding-32'>{$_GET['stmt']} in {$_GET['ctx']}</h5>\n";
		
		// firstly, invoke the ATPSiteTools to check that the
		// input statement is sound
		$stmt = escapeshellarg($_GET['stmt']);
		$ctx = escapeshellarg($_GET['ctx']);
		exec("\"..\Output\ATPSiteTools_Releasex64.exe\" --db ../Data/DB/eqlogic.db --ctx {$ctx} --norm {$stmt} 2>&1", $output, $ret_val);
		
		if ($ret_val == -1)
		{
			// this is bad and shouldn't really happen
			echo "Unexpected error in ATPSiteTools.";
			exit();
		}
		else if ($ret_val == 1)
		{
			// this means it's the user's fault
			echo "<p class=\"w3-text-grey\">Oops, search failed: the statement '{$_GET['stmt']}' was invalid.</h5>";
			exit();
		}
		// else the statement is valid, and normalised:
		$stmt = $output[0];

		echo "<p class='w3-text-grey'>This statement is normalised to {$stmt}.</p>";
		
		// connect to database
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// find theorem
		$prep = $db->prepare("SELECT id FROM theorems JOIN model_contexts ON ctx = ctx_id WHERE stmt = :stmt AND name = :ctx_name");
		$prep->bindValue(':stmt', $stmt, SQLITE3_TEXT);
		$prep->bindValue(':ctx_name', $_GET['ctx'], SQLITE3_TEXT);
		$query = $prep->execute();

		// if theorem exists
		if ($row = $query->fetchArray())
		{
			$thm_id = $row['id'];

			// does it have a proof?
			$query = $db->query("SELECT proof, is_axiom, proof_date FROM proofs WHERE thm_id={$thm_id}");
			echo "<p class=\"w3-text-grey\">";
			
			$had_proof = false;
			if ($row = $query->fetchArray())
			{
				$had_proof = true;
				
				// is it an axiom?
				if ($row['is_axiom'] == 1)
				{
					echo "This statement is an axiom.";
				}
				else
				{
					// format the proof nicely, in particular we need
					// to replace newlines with <br>s
					$pf = str_replace("\n", "<br>Is implied by ", $row['proof']);
					echo "Proof:</p>\n<p class=\"w3-text-grey\">Our target {$pf}<br>And the last line is trivial (follows from reflexivity, an axiom, or an existing theorem).";
				}
			}
			else
			{
				echo "";
			}
			echo "</p>\n";
			
			// if it had a proof, what theorems were used in that proof?
			if ($had_proof)
			{
				$query = $db->query("SELECT stmt FROM theorems JOIN theorem_usage ON used_thm_id = id WHERE target_thm_id = {$thm_id}");
				
				// list theorems used in proof
				$any_available = false;
				while ($row = $query->fetchArray())
				{
					// output an explanation paragraph on the first
					// iteration
					if (!$any_available)
					{
						echo "<p class=\"w3-text-grey\">Theorems at hand for the above proof:</p>\n<ul class=\"w3-ul\">\n";
					}
					$any_available = true;
					echo "<li class=\"w3-text-grey\"><a href=\"examine_thm.php?stmt={$row['stmt']}&ctx={$_GET['ctx']}\">{$row['stmt']}</a></li>\n";
				}
				echo "</ul>\n";	
				
				if (!$any_available)
				{
					echo "<p class=\"w3-text-grey\">No theorems were used to help prove the statement, only axioms.</p>";
				}
			}

			// get information about the proof attempts:
			$query = $db->query("SELECT name, time_cost, max_mem, num_expansions, DATE(attempt_date) as attempt_date FROM proof_attempts NATURAL JOIN search_settings WHERE thm_id = {$thm_id}");

			// for each proof attempt
			$attempted = false;
			while ($row = $query->fetchArray())
			{
				// output start of the table tag on first iteration
				if (!$attempted)
				{
					echo "<br><p class=\"w3-text-grey\">Here is a table of all the attempts at proving {$stmt}</p>\n";
					echo "<br><table class=\"w3-text-grey w3-table-all\"><tr><th>Search Settings</th><th>Time cost (s)</th><th>Maximum number of nodes in memory</th><th>Number of expansions</th><th>Date attempted</th></tr>\n";
				}
				$attempted = true;
				
				// output data about each attempt
				echo "<tr><td>{$row['name']}</td><td>{$row['time_cost']}</td><td>{$row['max_mem']}</td><td>{$row['num_expansions']}</td><td>{$row['attempt_date']}</td></tr>";
			}
			
			if ($attempted)
			{
				// end table
				echo "</table>\n";
			}
			else
			{
				// indicate that there have been no attempts
				echo "<br><p class=\"w3-text-grey\">No attempts have been made to try to prove {$stmt} yet.</p>\n";
			}

			// if this theorem is true, where has it been used?
			if ($had_proof)
			{
				$query = $db->query("SELECT stmt FROM theorems JOIN theorem_usage ON id = target_thm_id WHERE used_thm_id = {$thm_id}");
				
				$used = false;
				while ($row = $query->fetchArray())
				{
					if (!$used)
					{				
						echo "<br><p class=\"w3-text-grey\">{$stmt} has been used in proofs of the following statements:</p>\n";
						echo "<ul class=\"w3-ul\">\n";
					}
					$used = true;
					echo "<li class=\"w3-text-grey\"><a href=\"examine_thm.php?stmt={$row['stmt']}&ctx={$_GET['ctx']}\">{$row['stmt']}</a></li>";
				}
				echo "</ul>\n";
				
				if (!$used)
				{
					echo "<br><p class=\"w3-text-grey\">{$stmt} has not been used to help prove anything else yet.</p>\n";
				}
			}

		}
		else
		{
			// theorem didn't exist
			echo "<p class=\"w3-text-grey\">The statement \"{$stmt}\" was not found in the database (perhaps you're looking in the wrong context?).</p>";
		}
		?>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
