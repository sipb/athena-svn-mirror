#!/bin/sh
# $Id: revert-to-dash.sh,v 1.1 2001-05-06 15:53:29 ghudson Exp $

msg="Selecting yes will cause future logins to use the old Athena interface"
msg="$msg with the dash menu bar at the top and the mwm default window"
msg="$msg manager.  Are you sure you want to do this?"

if gdialog --yesno "$msg" 0 50; then
  touch $HOME/.athena_dash_interface
fi
