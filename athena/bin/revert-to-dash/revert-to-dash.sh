#!/bin/sh
# $Id: revert-to-dash.sh,v 1.1.6.1 2003-09-15 19:51:30 ghudson Exp $

msg="
  Selecting yes will cause future logins to use the old Athena  
  interface with the dash menu bar at the top and the mwm default  
  window manager.  Are you sure you want to do this?  
"

if gdialog --yesno "$msg" 5 50; then
  touch $HOME/.athena_dash_interface
fi
