#!/bin/sh
# $Id: finish-update-wrapper.sh,v 1.1 2004-03-31 00:38:13 rbasch Exp $

# This script runs the "real" finish-update, tee'ing output to the
# update log, and executes the end-of-update actions, culminating
# in reboot.  This was formerly done by /etc/init.d/finish-update
# itself; since that latter script may now be overwritten during
# finish-update, it now exec's this script.

method=`awk '{a=$6} END {print a}' /etc/athena/version`
newvers=`awk '{a=$7} END {print a}' /etc/athena/version`

sh /srvd/usr/athena/lib/update/finish-update "$newvers" 2>&1 \
  | tee -a /var/athena/update.log
if [ "$method" = Manual ]; then
  echo "The update to version $newvers is complete.  You"
  echo "may now examine the system before rebooting it"
  echo "under $newvers.  When you are finished, type"
  echo "'exit' and the system will reboot.  The shell"
  echo "prompt below is for a /bin/athena/tcsh process,"
  echo "regardless of what root's shell normally is."
  echo ""
  /bin/athena/tcsh
fi

echo "Update completed, rebooting in 15 seconds."
sync
sleep 15
exec reboot
