#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.43 1999-10-01 13:43:41 rbasch Exp $

trap "" 1 15

PATH=/bin:/bin/athena:/usr/bin:/usr/sbin:/usr/ucb; export PATH
HOSTTYPE=`/bin/athena/machtype`; export HOSTTYPE

# Usage: nuke directoryname
# Do the equivalent of rm -rf directoryname/*, except using saferm.
nuke()
{
	(
		cd $1
		if [ $? -eq 0 ]; then
			find * ! -type d -exec saferm {} \;
			find * -depth -type d -exec rmdir {} \;
		fi
	)
}

umask 22
. /etc/athena/rc.conf

# Set various flags (based on environment and command-line)
if [ "$1" = -detach ]; then
	dflags=""
else
	dflags="-clean"
fi

if [ "$1" = -prelogin ]; then
	if [ "$PUBLIC" = "false" ]; then
		exit 0;
	fi
	echo "Cleaning up..." >> /dev/console
	full=false
else
	full=true
fi

if [ -z "$USER" ]; then
	exec 1>/dev/console 2>&1
	quiet=-q
else
	echo "Reactivating workstation..."
	quiet=""
fi

# Flush all NFS uid mappings
/bin/athena/fsid $quiet -p -a

# Tell the Zephyr hostmanager to reset state
if [ -f /var/athena/zhm.pid -a "$ZCLIENT" = true ] ; then 
	/bin/kill -HUP `/bin/cat /var/athena/zhm.pid`
fi

# Zero any ticket files in /tmp that may have escaped other methods
# of destruction, before we clear /tmp. We must cd there since saferm
# will not follow symbolic links.
(cd /tmp; saferm -z tkt* krb5cc*) > /dev/null 2>&1

# For some reason, emacs leaves behind a lock file sometimes.  Nuke it.
rm -f /var/tmp/!!!SuperLock!!!

if [ "$full" = true ]; then
	# Clean temporary areas (including temporary home directories)
	case "$HOSTTYPE" in
	sun4)
		cp -p /tmp/ps_data /var/athena/ps_data
		nuke /tmp > /dev/null 2>&1
		cp -p /var/athena/ps_data /tmp/ps_data
		rm -f /var/athena/ps_data
		;;
	*)
		nuke /tmp > /dev/null 2>&1
		;;
	esac
	nuke /var/athena/tmphomedir > /dev/null 2>&1
fi

# Copy in latest password file
if [ "$PUBLIC" = true ]; then
	if [ -r /srvd/etc/passwd ]; then
		cp -p /srvd/etc/passwd /etc/passwd.local
		chmod 644 /etc/passwd.local
		chown root /etc/passwd.local
	fi
	if [ -r /srvd/etc/shadow ]; then
		cp -p /srvd/etc/shadow /etc/shadow.local
		chmod 600 /etc/shadow.local
		chown root /etc/shadow.local
	fi
	rm -rf /etc/athena/access >/dev/null 2>&1
fi

# Restore password and group files
if [ -s /etc/passwd.local ] ; then
	cmp -s /etc/passwd.local /etc/passwd || {
		cp -p /etc/passwd.local /etc/ptmp &&
		/bin/mv -f /etc/ptmp /etc/passwd &&
		sync
	}
fi
if [ -s /etc/shadow.local ] ; then
	cmp -s /etc/shadow.local /etc/shadow || {
		cp -p /etc/shadow.local /etc/stmp &&
		/bin/mv -f /etc/stmp /etc/shadow &&
		sync
	}
fi
if [ -s /etc/group.local ] ; then
	cmp -s /etc/group.local /etc/group || {
		cp -p /etc/group.local /etc/gtmp &&
		/bin/mv -f /etc/gtmp /etc/group &&
		sync
	}
fi

if [ "$full" = true ]; then
	# Reconfigure AFS state
	if [ "$AFSCLIENT" != "false" ]; then
		/etc/athena/config_afs > /dev/null 2>&1 &
	fi
fi

# Punt any processes owned by users not in /etc/passwd.
/etc/athena/cleanup -passwd

if [ "$full" = true ]; then
	# Remove session files.
	for i in /var/athena/sessions/*; do
		# Sanity check.
		if [ -s $i ]; then
			logger -p user.notice "Non-empty session record $i"
		fi
		rm -f $i
	done

	# Detach all remote filesystems
	/bin/athena/detach -O -h -n $quiet $dflags -a

	# Now start activate again
	/etc/athena/save_cluster_info

	if [ -f /var/athena/clusterinfo.bsh ] ; then
		. /var/athena/clusterinfo.bsh
	elif [ "$RVDCLIENT" = true ]; then
		echo "Can't determine system packs location."
		exit 1
	fi

	if [ "$RVDCLIENT" = true ]; then
		/bin/athena/attach	$quiet -h -n -o hard -O $SYSLIB
	fi

	# Perform an update if appropriate
	/srvd/auto_update reactivate

	if [ "$PUBLIC" = true -a -f /srvd/.rvdinfo ]; then
		NEWVERS=`awk '{a=$5} END{print a}' /srvd/.rvdinfo`
		THISVERS=`awk '{a=$5} END{print a}' /etc/athena/version`
		if [ "$NEWVERS" = "$THISVERS" ]; then
			/usr/athena/etc/track
			cf=`cat /srvd/usr/athena/lib/update/configfiles`
			for i in $cf; do
				if [ -f /srvd$i ]; then
					cp -p /srvd$i $i
				else
					cp -p /os$i $i
				fi
			done
			ps -e | awk '$4=="inetd" {print $1}' | xargs kill -HUP
		fi
		rm -f /etc/athena/reactivate.local /etc/ssh_*
	fi
fi

if [ "$ACCESSON" = true -a -f /usr/athena/bin/access_on ]; then
	/usr/athena/bin/access_on
elif [ "$ACCESSON" != true -a -f /usr/athena/bin/access_off ]; then
	/usr/athena/bin/access_off
fi

if [ "$full" = true ]; then
	if [ -f /etc/athena/reactivate.local ]; then
		/etc/athena/reactivate.local
	fi
fi

exit 0
