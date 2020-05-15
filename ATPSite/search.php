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
    <a href="search.php" class="w3-bar-item w3-button w3-hide-small w3-padding-large w3-white">Search</a>
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
      <h1>Search for a statement</h1>
	  <form action="examine_thm.php" method=post>
		<br>
		What statement are you looking for?
		<br>
		<input type=string name="stmt" />
		<br>
		<br>
		<input type=submit value="Go">
		</form>
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
