#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.53 2000-09-18 19:29:05 jweiss Exp $

trap "" 1 15

PATH=/bin:/etc/athena:/bin/athena:/usr/bin:/usr/sbin:/usr/ucb:/usr/bsd; export PATH
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
if [ -f /var/athena/clusterinfo.bsh ] ; then
	. /var/athena/clusterinfo.bsh
fi

# Determine where the congfig files live
THISVERS=`awk '{a=$5} END{print a}' /etc/athena/version`
if [ -n "$SYSPREFIX" ]; then
	config=$SYSPREFIX/config/$THISVERS
else
	config=/srvd
fi

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
	if [ sun4 = "$HOSTTYPE" -a -f /tmp/ps_data ]; then
		cp -p /tmp/ps_data /var/athena/ps_data
		nuke /tmp > /dev/null 2>&1
		cp -p /var/athena/ps_data /tmp/ps_data
		rm -f /var/athena/ps_data
	else
		nuke /tmp > /dev/null 2>&1
	fi
	nuke /var/athena/tmphomedir > /dev/null 2>&1
fi

# Copy in latest password file
if [ "$PUBLIC" = true ]; then
	if [ -r $config/etc/passwd ]; then
		syncupdate -c /etc/passwd.local.new $config/etc/passwd \
			/etc/passwd.local
	fi
	if [ -r $config/etc/shadow ]; then
		syncupdate -c /etc/shadow.local.new $config/etc/shadow \
			/etc/shadow.local
	fi
	if [ -r $config/etc/group ]; then
		cp -p $config/etc/group /etc/group.local
		chmod 644 /etc/group.local
		chown root /etc/group.local
	fi
	rm -rf /etc/athena/access >/dev/null 2>&1
fi

# Restore password and group files
if [ -s /etc/passwd.local ] ; then
	syncupdate -c /etc/passwd.new /etc/passwd.local /etc/passwd
fi
if [ -s /etc/shadow.local ] ; then
	syncupdate -c /etc/shadow.new /etc/shadow.local /etc/shadow
fi
if [ -s /etc/group.local ] ; then
	syncupdate -c /etc/group.new /etc/group.local /etc/group
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
		/bin/athena/attach $quiet -h -n -O $SYSLIB
	fi

	# Perform an update if appropriate
	update_ws -a reactivate

	if [ "$PUBLIC" = true -a -f /srvd/.rvdinfo ]; then
		NEWVERS=`awk '{a=$5} END{print a}' /srvd/.rvdinfo`
		if [ "$NEWVERS" = "$THISVERS" ]; then
			/usr/athena/etc/track
			cf=`cat /srvd/usr/athena/lib/update/configfiles`
			for i in $cf; do
				if [ -f /srvd$i ]; then
					src=/srvd$i
				else
					src=/os$i
				fi
				syncupdate -c $i.new $src $i
			done
			ps -e | awk '$4=="inetd" {print $1}' | xargs kill -HUP
		fi
	fi
	if [ "$PUBLIC" = true ]; then
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
