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
      <h1>Submit a task</h1>
	  <form action="submitted.php" method=post>
		<br>
		Statement:
		<input type=string name="stmt" />
		<br>
		Context name:
		<input type=string name="ctx" />
		<br>
		Deadline (DD/MM/YYYY):
		<input type=string name="deadline" />
		<br>
		Priority:
		<input type=float name="priority" />
		<br>
		<br>
		<input type=submit value="Go">
		</form>
    </div>
  </div>
</div>

<!--#include file="footer.php" -->

</body>
</html>
