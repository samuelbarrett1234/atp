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
		echo "<h5 class='w3-padding-32'>{$_GET['stmt']}</h5>\n";

		// connect to database
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// find theorem
		$prep = $db->prepare("SELECT id FROM theorems WHERE stmt = :stmt");
		$prep->bindValue(':stmt', $_GET['stmt'], SQLITE3_TEXT);
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
				echo "<p class=\"w3-text-grey\">Theorems at hand for the above proof:</p>\n<ul class=\"w3-ul\">\n";

				$query = $db->query("SELECT stmt FROM theorems JOIN theorem_usage ON used_thm_id = id WHERE target_thm_id = {$thm_id}");
				
				// list theorems used in proof
				$any_available = false;
				while ($row = $query->fetchArray())
				{
				  $any_available = true;
				  echo "<li class=\"w3-text-grey\"><a href=\"examine_thm.php?stmt={$row['stmt']}\">{$row['stmt']}</a></li>\n";
				}
				echo "</ul>\n";	
				
				if (!$any_available)
				{
					echo "<p class=\"w3-text-grey\">No theorems were used to help prove the statement, only axioms.</p>";
				}
			}

			// get information about the proof attempts:
			$query = $db->query("SELECT name, time_cost, max_mem, num_expansions, attempt_date FROM proof_attempts NATURAL JOIN search_settings WHERE thm_id = {$thm_id}");

			// for each proof attempt
			$attempted = false;
			while ($row = $query->fetchArray())
			{
				// output start of the table tag on first iteration
				if (!$attempted)
				{
					echo "<br><p class=\"w3-text-grey\">Here is a table of all the attempts at proving {$_GET['stmt']}</p>\n";
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
				echo "<br><p class=\"w3-text-grey\">No attempts have been made to try to prove {$_GET['stmt']} yet.</p>\n";
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
						echo "<br><p class=\"w3-text-grey\">{$_GET['stmt']} has been used in proofs of the following statements:</p>\n";
						echo "<ul class=\"w3-ul\">\n";
					}
					$used = true;
					echo "<li class=\"w3-text-grey\"><a href=\"examine_thm.php?stmt={$row['stmt']}\">{$row['stmt']}</a></li>";
				}
				echo "</ul>\n";
				
				if (!$used)
				{
					echo "<br><p class=\"w3-text-grey\">{$_GET['stmt']} has not been used to help prove anything else yet.</p>\n";
				}
			}

		}
		else
		{
			// theorem didn't exist
			echo "<p class=\"w3-text-grey\">The statement \"{$_GET['stmt']}\" was not found in the database.</p>";
		}
	  ?>
    </div>
  </div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
