#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Header: /afs/dev.mit.edu/source/repository/packs/maint/reactivate.sh,v 1.5 1991-02-06 14:44:22 probe Exp $

trap "" 1 15

PATH=/bin:/usr/ucb:/usr/bin; export PATH

umask 22
. /etc/rc.conf

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
if [ -f /afs/athena.mit.edu/service/CellServDB ] ; then
	${cp} /afs/athena.mit.edu/service/CellServDB /usr/vice/etc/Ctmp && \
	/bin/mv -f /usr/vice/etc/Ctmp /usr/vice/etc/CellServDB.public
fi
if [ -f /afs/athena.mit.edu/service/aklog ] ; then
	${cp} /afs/athena.mit.edu/service/aklog \
		   /bin/athena/aklog
fi

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
	if [ "${USRLIB}" = "" ]; then
		/bin/athena/attach	$quiet -h -n -o hard  $SYSLIB
	else
		/bin/athena/attach	$quiet -h -n -o hard  $SYSLIB \
					$quiet -h -n -o hard  $USRLIB
	fi
fi

# Perform an update if appropriate
if [ -f /srvd/auto_update ] ; then 
	/srvd/auto_update reactivate
fi

if [ -f /usr/athena/access_off ]; then /usr/athena/access_off; fi

if [ -f /etc/athena/reactivate.local ]; then
	/etc/athena/reactivate.local
fi

exit 0
