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
		<h1>Search for a statement</h1>
		<form action="examine_thm.php" method=get>
		<br>
		What statement are you looking for?
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
		<input type=submit value="Go" class="w3-input w3-button">
		</form>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
