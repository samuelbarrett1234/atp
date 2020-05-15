# ATPSite Readme

This folder contains all of the HTML/PHP files for the ATP website, allowing users to post tasks and to read information about existing proofs.

## Setup

I used `xampp` and Apache web server to run the PHP files. I had to add the following to my `httpd.conf`:

```

<Directory "C:/Users/Samue/source/repos/atp/ATPSite">
		Options Indexes FollowSymLinks MultiViews
		Options +Includes
		AddType text/html .php
		AddOutputFilter INCLUDES .php
		AllowOverride All
		Require all granted
</Directory>

Alias /atp "C:/Users/Samue/source/repos/atp/ATPSite"

```