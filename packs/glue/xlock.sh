#!/bin/sh
# $Id: xlock.sh,v 1.3 1998-07-31 19:38:43 ghudson Exp $

if [ -r /etc/athena/rc.conf ]; then
	. /etc/athena/rc.conf
	if [ "$PUBLIC" != true ]; then
		exec xlock.real "$@"
	fi
fi

echo "xlock is not appropriate for cluster workstations, because"
echo "it does not display an elapsed time or put up a button to"
echo "allow others to log you out after a set time."
echo ""
echo "If you really wish to run xlock, run it as 'xlock.real' instead."
echo "But if you do so, others may legitimately reboot the machine at"
echo "any time to log you out."
echo ""
echo "Press return to read the On-Line Consulting stock answer about"
echo "screensavers, or press Control-C to go back to the prompt."
read dummy
/bin/athena/attachandrun infoagents htmlview htmlview \
	http://web.mit.edu/answers/workstations/other_xscreensaver.html
