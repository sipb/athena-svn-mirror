#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.6 1991-07-19 15:42:28 probe Exp $

trap "" 1 15

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin; export PATH

umask 22
. /etc/athena/rc.conf

case "${SYSTEM}" in
ULTRIX*)
	cp=/bin/cp
	;;
*)
	cp="/bin/cp -p"
esac

if [ "${USER}" = "" ]; then
	exec 1>/dev/console 2>&1
	quiet=-q
else
	echo Reactivating workstation...
	quiet=
fi

# Default options
dflags="-clean"

# Parse command line arguments (we only accept one for now -detach)
if [ "$1" = "-detach" ]; then dflags=""; fi

# Flush all NFS uid mappings
/bin/athena/fsid $quiet -p -a

# Tell the Zephyr hostmanager to reset state
if [ -f /etc/athena/zhm.pid ] ; then 
	/bin/kill -HUP `/bin/cat /etc/athena/zhm.pid`
fi

if [ "${MACHINE}" != "RSAIX" ]; then
# Then clean temporary areas (including temporary home directories)
    /bin/mv /tmp/.X11-unix /tmp/../.X11-unix
    /bin/rm -rf /tmp/ > /dev/null 2>&1
    /bin/mv /tmp/../.X11-unix /tmp/.X11-unix

# Next, restore password, group, and AFS-cell files
    if [ -f /etc/passwd.local ] ; then
	${cp} /etc/passwd.local /etc/ptmp && /bin/mv -f /etc/ptmp /etc/passwd
	/bin/rm -f /etc/passwd.dir /etc/passwd.pag
    fi
    if [ -f /etc/group.local ] ; then
	${cp} /etc/group.local /etc/gtmp && /bin/mv -f /etc/gtmp /etc/group
    fi
fi

if [ -f /afs/athena.mit.edu/service/aklog ] ; then
	${cp} /afs/athena.mit.edu/service/aklog \
		   /bin/athena/aklog
fi
/etc/athena/config_afs &

# punt any processes owned by users not in /etc/passwd
/etc/athena/cleanup -passwd

# Finally, detach all remote filesystems
/bin/athena/detach -O -h -n $quiet $dflags -a

# Now start activate again
/etc/athena/save_cluster_info

if [ -f /etc/clusterinfo.bsh ] ; then
	. /etc/clusterinfo.bsh
else
	if [ "${RVDCLIENT}" = "true" ]; then
		echo "Can't find library servers."
		exit 1
	fi
fi

if [ "${RVDCLIENT}" = "true" ]; then
	/bin/athena/attach	$quiet -h -n -o hard  $SYSLIB
fi

if [ "${AFSCLIENT}" = "false" ]; then
        awk '$2=="0+NFS" && $8=="/afs" {print $4}' \
                < /etc/attachtab > /etc/afs-nfs-host
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
