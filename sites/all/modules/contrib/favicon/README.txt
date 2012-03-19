Mike Howard (mikehoward) 2011-01-20
Drupal 7 Notes:

This module requires that the core, optional module 'Shortcut' be enabled AND
the line:
 RewriteCond %{REQUEST_URI} !=/favicon.ico
in '.htaccess' be deleted or commented out [which is a bad thing], as in
 # RewriteCond %{REQUEST_URI} !=/favicon.ico

