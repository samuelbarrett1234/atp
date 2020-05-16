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
		<h1>Try to prove a statement</h1>
		<form action="proving.php" method=post>
		<br>
		What statement do you want to try to prove?
		<br>
		<input type=string name="stmt" class="w3-input" placeholder="Enter the statement text" required />
		<br>
		Context name:
		<select name="ctx" class="w3-select">
			<?php
			$db = new SQLite3('../Data/DB/eqlogic.db');
			
			$query = $db->query("SELECT name FROM model_contexts");
			
			// enumerate the model contexts
			while ($row = $query->fetchArray())
			{
				echo "<option value=\"{$row['name']}\" class=\"w3-input w3-option\">{$row['name']}</option>";
			}
			?>
		</select>
		<br>
		<br>
		<input type=submit value="Prove" class="w3-input w3-button">
		</form>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
