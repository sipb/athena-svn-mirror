#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.19 1994-07-08 15:27:41 cfields Exp $

trap "" 1 15

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin; export PATH

umask 22
. /etc/athena/rc.conf

# Default options
dflags="-clean"


# Set various flags (based on environment and command-line)
if [ "$1" = "-detach" ]; then dflags=""; fi

if [ "$1" = "-prelogin" ]; then
	if [ "${PUBLIC}" = "false" ]; then exit 0; fi
	echo "Cleaning up..." >> /dev/console
else
	full=true
fi

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

# Sun attach bug workaround. Attach apparently loses on some options.
# fsid does not though, so we zap them only after executing fsid.
if [ "${MACHINE}" = "SUN4" ]; then
	dflags=""
	quiet=""
fi

# Tell the Zephyr hostmanager to reset state
if [ -f /etc/athena/zhm.pid ] ; then 
	/bin/kill -HUP `/bin/cat /etc/athena/zhm.pid`
fi

case "${MACHINE}" in
RSAIX)
	chmod 666 /dev/hft
	;;
*)
	;;
esac

# kdestroy from /tmp any ticket files that may have escaped other methods
# of destruction, before we clear /tmp.
export KRBTKFILE
for i in /tmp/tkt*; do
  KRBTKFILE=$i
  /usr/athena/bin/kdestroy -f
done
unset KRBTKFILE

if [ $full ]; then		# START tmp clean
# Clean temporary areas (including temporary home directories)
case "${MACHINE}" in
RSAIX)
	find /tmp -depth \( -type f -o -type l \) -print | xargs /bin/rm -f -
	find /tmp -depth -type d -print | xargs /bin/rmdir 1>/dev/null 2>&1
	;;

SUN4)
	cp -p /tmp/ps_data /usr/tmp/ps_data
	/bin/rm -rf /tmp/* > /dev/null 2>&1
	cp -p /usr/tmp/ps_data /tmp/ps_data
	/bin/rm -f /usr/tmp/ps_data
	;;
*)
	/bin/mv /tmp/.X11-unix /tmp/../.X11-unix
	/bin/rm -rf /tmp/ > /dev/null 2>&1
	/bin/mv /tmp/../.X11-unix /tmp/.X11-unix
	;;
esac
fi				# END tmp clean

# Restore password and group files
case "${MACHINE}" in
RSAIX)
	;;
SUN4)
	if [ -f /etc/passwd.local ] ; then
	    ${cp} /etc/passwd.local /etc/ptmp && /bin/mv -f /etc/ptmp /etc/passwd
	fi
	if [ -f /etc/shadow.local ] ; then
	    ${cp} /etc/shadow.local /etc/stmp && /bin/mv -f /etc/stmp /etc/shadow
	fi
	if [ -f /etc/group.local ] ; then
	    ${cp} /etc/group.local /etc/gtmp && /bin/mv -f /etc/gtmp /etc/group
	fi
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

if [ $full ]; then		# START AFS reconfig
# Reconfigure AFS state
if [ "${AFSCLIENT}" != "false" ]; then
    if [ -f /afs/athena.mit.edu/service/aklog ] ; then
	${cp} /afs/athena.mit.edu/service/aklog /bin/athena/aklog.new && \
	test -s /bin/athena/aklog.new && \
	/bin/mv /bin/athena/aklog.new /bin/athena/aklog
	/bin/rm -f /bin/athena/aklog.new
    fi
    /etc/athena/config_afs > /dev/null 2>&1 &
fi
fi				# END AFS reconfig

# punt any processes owned by users not in /etc/passwd
/etc/athena/cleanup -passwd

if [ $full ]; then		# START time-consuming stuff
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
if [ "${AUTOUPDATE}" = "true" -a -f /srvd/auto_update ]; then 
	/srvd/auto_update reactivate
elif [ -f /srvd/.rvdinfo ]; then
	NEWVERS=`awk '{a=$5}; END{print a}' /srvd/.rvdinfo`
	VERSION=`awk '{a=$5}; END{print a}' /etc/athena/version`
	if [ "${NEWVERS}" != "${VERSION}" ]; then
		cat <<EOF
The workstation software version ($VERSION) does not match the
version on the system packs ($NEWVERS).  A new version of software
may be available.  Please contact Athena Operations (x3-1410) to
have your workstation updated.
EOF
		if [ ! -f /usr/tmp/update.check -a -f /usr/ucb/logger ]; then
			/usr/ucb/logger -t `hostname` -p user.notice at revision $VERSION
			cp /dev/null /usr/tmp/update.check
		fi
	fi
fi
fi				# END time-consuming stuff

if [ -f /usr/athena/bin/access_off ]; then /usr/athena/bin/access_off; fi

if [ $full ]; then		# START reactivate.local
if [ -f /etc/athena/reactivate.local ]; then
	/etc/athena/reactivate.local
fi
fi				# END reactivate.local

exit 0
