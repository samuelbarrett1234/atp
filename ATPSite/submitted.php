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
			<?php
			// firstly, invoke the ATPSiteTools to check that the
			// input statement is sound
			$stmt = escapeshellarg($_POST['stmt']);
			$ctx = escapeshellarg($_POST['ctx']);
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
				echo "<h1>Submission failed</h1>";
				echo "<h5 class=\"w3-padding-32\">The statement '{$_POST['stmt']}' was invalid.</h5>";
				exit();
			}
			// else the statement is valid, and normalised:
			$stmt = $output[0];
			
			// connect to database
			$db = new SQLite3('../Data/DB/eqlogic.db');
			
			// convert deadline
			$deadline_formatted = DateTime::createFromFormat('d/m/Y',
				$_POST['deadline']);
			if ($deadline_formatted === false)
			{
				// failed - bad date!
				echo "<h1>Submission failed</h1>";
				echo "<h5 class=\"w3-padding-32\">The deadline '{$_POST['deadline']}' was invalid.</h5>";
				exit();
			}
			$deadline = unixtojd($deadline_formatted->getTimestamp());
			
			// create theorem
			$prep = $db->prepare("INSERT OR IGNORE INTO theorems(stmt, ctx_id) VALUES (:stmt, (SELECT ctx_id FROM model_contexts WHERE name = :ctx_name))");
			$prep->bindValue(':stmt', $stmt, SQLITE3_TEXT);
			$prep->bindValue(':ctx_name', $_POST['ctx'], SQLITE3_TEXT);
			$prep->execute();
			
			// create task
			$prep = $db->prepare("INSERT OR IGNORE INTO tasks (thm_id, deadline, priority) VALUES ((SELECT thm_id FROM theorems NATURAL JOIN model_contexts WHERE stmt = :stmt AND name = :ctx_name), :deadline, :priority)");
			$prep->bindValue(':stmt', $stmt, SQLITE3_TEXT);
			$prep->bindValue(':ctx_name', $_POST['ctx'], SQLITE3_TEXT);
			$prep->bindValue(':deadline', $deadline, SQLITE3_FLOAT);
			$prep->bindValue(':priority', $_POST['priority'], SQLITE3_FLOAT);
			$prep->execute();
			
			// if we get here, success:
			echo "<h1>Submission successful!</h1>";
			echo "<h5 class=\"w3-padding-32\">The task for {$_POST['stmt']} has been added to the database, and the ATP will get round to it at some point. The task was normalised to {$stmt}.</h5>";
			?>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
