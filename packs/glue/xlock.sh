#!/bin/sh
# $Id: xlock.sh,v 1.1 1998-06-08 14:11:16 ghudson Exp $

if [ -r /etc/athena/rc.conf ]; then
	. /etc/athena/rc.conf
	if [ "$PUBLIC" != true ]; then
		exec xlock.real "$@"
	fi
fi

echo "xlock is not appropriate for cluster workstations, because:"
echo ""
echo "	* Anyone can unlock the machine using the well-known root"
echo "	  password."
echo "	* It does not display an elapsed time or put up a button"
echo "	  to allow others to log you out after a set time."
echo ""
echo "If you really wish to run xlock, run it as 'xlock.real' instead."
echo ""
echo "Press return to read the On-Line Consulting stock answer about"
echo "screensavers, or press Control-C to go back to the prompt."
read dummy
/bin/athena/attachrun infoagents htmlview htmlview \
	http://web.mit.edu/answers/workstations/other_xscreensaver.html
