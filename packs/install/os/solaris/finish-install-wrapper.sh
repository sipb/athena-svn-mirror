#!/bin/sh
# $Id: finish-install-wrapper.sh,v 1.1 2004-05-25 16:50:36 rbasch Exp $

# This script runs the "real" finish-install, tee'ing output to the
# install log, and executes the end-of-install actions, culminating
# in reboot.

# We get one argument, the workstation version we're installing at.
vers="$1"

. /var/athena/install.vars

sh /srvd/install/finish-install "$vers" 2>&1 | tee -a /var/athena/install.log

case $REBOOT in
N)
  echo "The installation of Athena version $vers is complete."
  echo "You may now customize the system before rebooting it."
  echo "When you have finished, type \"exit\" to reboot."
  /sbin/sh
  ;;
esac

rm -f /var/athena/install.vars

echo "Install completed, rebooting now..."
sync
exec reboot
