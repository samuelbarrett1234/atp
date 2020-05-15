<!DOCTYPE html>
<html lang="en">
<title>ATP</title>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Lato">
<link rel="stylesheet" href="https://fonts.googleapis.com/css?family=Montserrat">
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
<link rel="stylesheet" type="text/css" href="style.css">

<!--
Template obtained from https://www.w3schools.com/w3css/w3css_templates.asp
-->

<body>

<!-- Navbar -->
<div class="w3-top">
  <div class="w3-bar w3-red w3-card w3-left-align w3-large">
    <a class="w3-bar-item w3-button w3-hide-medium w3-hide-large w3-right w3-padding-large w3-hover-white w3-large w3-red"
		href="javascript:void(0);" onclick="toggle_menu()" title="Toggle Navigation Menu"><i class="fa fa-bars"></i></a>
	
    <a href="index.php" class="w3-bar-item w3-button w3-hide-small w3-padding-large w3-hover-white">ATP</a>
    <a href="submit.php" class="w3-bar-item w3-button w3-hide-small w3-padding-large w3-hover-white">Submit Tasks</a>
    <a href="search.php" class="w3-bar-item w3-button w3-padding-large w3-white">Search</a>
  </div>
</div>

<!-- Header -->
<header class="w3-container w3-red w3-center" style="padding:128px 16px">
  <h1 class="w3-margin w3-jumbo">ATP</h1>
  <p class="w3-xlarge">An automated theorem prover personal project</p>
  <button class="w3-button w3-black w3-padding-large w3-large w3-margin-top">Get Started</button>
</header>

<!-- First Grid -->
<div class="w3-row-padding w3-padding-64 w3-container">
  <div class="w3-content">
    <div class="w3-twothird">
      <h1>Statement Info</h1>
	  <?php
		echo "<h5 class='w3-padding-32'>{$_POST['stmt']}</h5>\n";

		// connect to database
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// find theorem
		$prep = $db->prepare("SELECT id FROM theorems WHERE stmt = :stmt");
		$prep->bindValue(':stmt', $_POST['stmt'], SQLITE3_TEXT);
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
				
				$any_available = false;
				while ($row = $query->fetchArray())
				{
				  $any_available = true;
				  echo "<li class=\"w3-text-grey\">{$row['stmt']}</li>\n";
				}
				echo "</ul>\n";	
				
				if (!$any_available)
				{
					echo "<p class=\"w3-text-grey\">No theorems were used to help prove the statement, only axioms.</p>";
				}
			}

			// get information about the proof attempts:
			$query = $db->query("SELECT name, time_cost, max_mem, num_expansions, attempt_date FROM proof_attempts NATURAL JOIN search_settings WHERE thm_id = {$thm_id}");

			
			echo "<br><p class=\"w3-text-grey\">Here is a table of all the attempts at proving {$_POST['stmt']}</p>\n";
			echo "<br><table class=\"w3-text-grey w3-table-all\"><tr><th>Search Settings</th><th>Time cost (s)</th><th>Maximum number of nodes in memory</th><th>Number of expansions</th><th>Date attempted</th></tr>\n";

			while ($row = $query->fetchArray())
			{
			  // output data about each attempt
			  echo "<tr><td>{$row['name']}</td><td>{$row['time_cost']}</td><td>{$row['max_mem']}</td><td>{$row['num_expansions']}</td><td>{$row['attempt_date']}</td></tr>";
			}

			echo "</table>\n";

			// if this theorem is true, where has it been used?
			if ($had_proof)
			{
				echo "<br><p class=\"w3-text-grey\">{$_POST['stmt']} has been used in proofs of the following statements:</p>\n";

				$query = $db->query("SELECT stmt FROM theorems JOIN theorem_usage ON id = target_thm_id WHERE used_thm_id = {$thm_id}");
				
				echo "<ul class=\"w3-ul\">\n";
				
				$used = false;
				while ($row = $query->fetchArray())
				{
					$used = true;
					echo "<li class=\"w3-text-grey\">{$row['stmt']}</li>";
				}
				echo "</ul>\n";
				
				if (!$used)
				{
					echo "<p class=\"w3-text-grey\">{$_POST['stmt']} has not been used yet.</p>\n";
				}
			}

		}
		else
		{
			// theorem didn't exist
			echo "<p class=\"w3-text-grey\">The statement \"{$_POST['stmt']}\" was not found in the database.</p>";
		}
	  ?>
    </div>
  </div>
</div>

<!-- Footer -->
<footer class="w3-container w3-padding-64 w3-center w3-opacity">  
 <p>Checkout the repository <a href="https://github.com/samuelbarrett1234/atp" target="_blank">here</a></p>
</footer>

<script src="script.js" />
</body>
</html>
