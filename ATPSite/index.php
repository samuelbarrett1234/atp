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
	
    <a href="index.php" class="w3-bar-item w3-button w3-padding-large w3-white">ATP</a>
    <a href="submit.php" class="w3-bar-item w3-button w3-hide-small w3-padding-large w3-hover-white">Submit Tasks</a>
    <a href="search.php" class="w3-bar-item w3-button w3-hide-small w3-padding-large w3-hover-white">Search</a>
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
      <h1>Automated Theorem Proving</h1>
      <h5 class="w3-padding-32">This automated theorem prover is given several sets of axioms to work in (called "contexts") and the user can submit statements in a given context to the ATP, which then tries to determine if that statement can be deduced from the axioms, or not. A priori, the theorem prover <b>has no way to determine if a statement is deducible from the axioms or not</b>, as this problem is clearly undecidable in general. However, if the given set of axioms is complete, the theorem prover will always be able to find a proof that either the statement is deducible from the axioms, or its negation is.</h5>

      <p class="w3-text-grey">As of yet, the theorem prover only works in a simplified version of <b>equational logic</b>, however first-order logic is on the roadmap. The equational logic version that the theorem prover works in has the following limitations: every variable is universally quantified, and there are no boolean operators (and / or / not). The only predicate is equality. Hence the only questions you can ask are of the form: "For all x, y, z, ..., is this expression equal to this expression?". Since there is no negation, it is <b>impossible to disprove a statement</b> in this implementation of equational logic.</p>
	  
	  <p class="w3-text-grey">Due to the undecidability of the problem being tackled, the aim of this project is to take a more statistical-based approach to theorem proving, where proofs are searched for using statistical models. (However of course, if a proof is obtained, it is guaranteed to be correct.) Furthermore, for simplification, no attempt is made to make the generated proofs readable. Instead, one is more interested in whether a statement has a proof, than the proof itself.</p>
    </div>
  </div>
</div>

<!-- Second Grid -->
<div class="w3-row-padding w3-light-grey w3-padding-64 w3-container">
  <div class="w3-content">
    <div class="w3-twothird">
      <h1>Prover Statistics</h1>
      <h5 class="w3-padding-32">Here is what the prover has done so far:</h5>
	  <ul class="w3-text-grey">
	  <?php
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		$query = $db->query("SELECT count(stmt) as n FROM theorems WHERE EXISTS (SELECT 1 FROM proofs WHERE thm_id = id)");
		$row = $query->fetchArray();
		echo "<li>Number of proven theorems: {$row['n']}</li>";
		
		$query = $db->query("SELECT count(stmt) as n FROM theorems WHERE NOT EXISTS (SELECT 1 FROM proofs WHERE thm_id = id)");
		$row = $query->fetchArray();
		echo "<li>Number of unproven theorems: {$row['n']}</li>";
		
		$query = $db->query("SELECT count(*) as n, sum(time_cost) as t, sum(num_expansions) as e FROM proof_attempts");
		$row = $query->fetchArray();
		
		echo "<li>Total number of proof attempts: {$row['n']}</li>";
		echo "<li>Total computation time: {$row['t']}</li>";
		echo "<li>Total number of search tree node expansions: {$row['e']}</li>";
	  ?>
	  </ul>
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
