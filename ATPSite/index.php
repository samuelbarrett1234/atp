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
		<ul class="w3-ul w3-text-grey">
		<?php
		$db = new SQLite3('../Data/DB/eqlogic.db');
		
		// get number of proven theorems
		$query = $db->query("SELECT count(stmt) as n FROM theorems WHERE EXISTS (SELECT 1 FROM proofs WHERE thm_id = id)");
		$row = $query->fetchArray();
		echo "<li>Number of proven theorems: {$row['n']}</li>";
		
		// get number of unproven theorems
		$query = $db->query("SELECT count(stmt) as n FROM theorems WHERE NOT EXISTS (SELECT 1 FROM proofs WHERE thm_id = id)");
		$row = $query->fetchArray();
		echo "<li>Number of unproven theorems: {$row['n']}</li>";
		
		// get number of proof attempts, total time cost, and total
		// expansions
		$query = $db->query("SELECT count(*) as n, sum(time_cost) as t, sum(num_expansions) as e FROM proof_attempts");
		$row = $query->fetchArray();
		
		echo "<li>Total number of proof attempts: {$row['n']}</li>";
		echo "<li>Total computation time: {$row['t']}s</li>";
		echo "<li>Total number of search tree node expansions: {$row['e']}</li>";
		?>
		</ul>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
