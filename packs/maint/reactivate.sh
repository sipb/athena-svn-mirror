#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.10 1991-08-26 04:00:50 probe Exp $

trap "" 1 15

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin; export PATH

umask 22
. /etc/athena/rc.conf

# Default options
dflags="-clean"


# Set various flags (based on environment and command-line)
if [ "$1" = "-detach" ]; then dflags=""; fi

if [ "${USER}" = "" ]; then
	exec 1>/dev/console 2>&1
	quiet=-q
else
	echo Reactivating workstation...
	quiet=
fi

case "${SYSTEM}" in
ULTRIX*)
	cp=/bin/cp
	;;
*)
	cp="/bin/cp -p"
esac



# Flush all NFS uid mappings
/bin/athena/fsid $quiet -p -a

# Tell the Zephyr hostmanager to reset state
if [ -f /etc/athena/zhm.pid ] ; then 
	/bin/kill -HUP `/bin/cat /etc/athena/zhm.pid`
fi

# Clean temporary areas (including temporary home directories)
case "${MACHINE}" in
RSAIX)
	find /tmp -depth \( -type f -o -type l \) -print | xargs /bin/rm -f -
	find /tmp -depth -type d -print | xargs /bin/rmdir 1>/dev/null 2>&1
	;;
*)
	/bin/mv /tmp/.X11-unix /tmp/../.X11-unix
	/bin/rm -rf /tmp/ > /dev/null 2>&1
	/bin/mv /tmp/../.X11-unix /tmp/.X11-unix
	;;
esac

# Restore password and group files
case "${MACHINE}" in
RSAIX)
	;;
*)
	if [ -f /etc/passwd.local ] ; then
	    ${cp} /etc/passwd.local /etc/ptmp && /bin/mv -f /etc/ptmp /etc/passwd
	    /bin/rm -f /etc/passwd.dir /etc/passwd.pag
	fi
	if [ -f /etc/group.local ] ; then
	    ${cp} /etc/group.local /etc/gtmp && /bin/mv -f /etc/gtmp /etc/group
	fi
	;;
esac

# Reconfigure AFS state
if [ -f /afs/athena.mit.edu/service/aklog ] ; then
	${cp} /afs/athena.mit.edu/service/aklog \
		   /bin/athena/aklog
fi
/etc/athena/config_afs > /dev/null 2>&1 &

# punt any processes owned by users not in /etc/passwd
/etc/athena/cleanup -passwd

# Finally, detach all remote filesystems
/bin/athena/detach -O -h -n $quiet $dflags -a

# Now start activate again
/etc/athena/save_cluster_info

if [ -f /etc/athena/clusterinfo.bsh ] ; then
	. /etc/athena/clusterinfo.bsh
else
	if [ "${RVDCLIENT}" = "true" ]; then
		echo "Can't find library servers."
		exit 1
	fi
fi

if [ "${RVDCLIENT}" = "true" ]; then
	/bin/athena/attach	$quiet -h -n -o hard  $SYSLIB
fi

# Perform an update if appropriate
if [ -f /srvd/auto_update ] ; then 
	/srvd/auto_update reactivate
fi

if [ -f /usr/athena/bin/access_off ]; then /usr/athena/bin/access_off; fi

if [ -f /etc/athena/reactivate.local ]; then
	/etc/athena/reactivate.local
fi

exit 0
