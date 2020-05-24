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
		<h1>Proof results</h1>
		<?php
		$output = null;
		
		// extract post arguments
		$stmt = escapeshellarg('"' . $_POST['stmt'] . '"');
		$ctx = escapeshellarg($_POST['ctx']);
		
		// execute proof command
		exec("\"..\Output\atp_Releasex64.exe\" --db ../Data/Queries/dbconfig.json --ctx {$ctx} --ss ids-informed-tiny --prove {$stmt} 2>&1", $output);
		
		if ($output ==  null)
		{
			// error
			echo "<p class=\"w3-text-grey\">Oops, an error occurred.</p>\n";
		}
		else
		{
			// concat array
			$output = implode("</p><p class=\"w3-text-grey\">", $output);
			
			echo "<p class=\"w3-text-grey\">{$output}</p>\n";
			
			echo "<p class=\"w3-text-grey\">Note that, if your proof failed due to time constraints, this may be because the website interface only allows you to make proof attempts using the lowest settings.</p>\n";
		}
		?>
		</div>
	</div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
