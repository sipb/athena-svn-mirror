#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.33 1997-12-02 20:23:15 jweiss Exp $

trap "" 1 15

PATH=/bin:/bin/athena:/usr/ucb:/usr/bin; export PATH
HOSTTYPE=`/bin/athena/machtype`; export HOSTTYPE

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

# kdestroy from /tmp any ticket files that may have escaped other methods
# of destruction, before we clear /tmp.
for i in /tmp/tkt* /tmp/krb5cc*; do
	KRBTKFILE=$i /usr/athena/bin/kdestroy -f
done

if [ "$full" = true ]; then
	# Clean temporary areas (including temporary home directories)
	case "$HOSTTYPE" in
	sun4)
		cp -p /tmp/ps_data /var/athena/ps_data
		rm -rf /tmp/* /tmp/.[^.]* > /dev/null 2>&1
		cp -p /var/athena/ps_data /tmp/ps_data
		rm -f /var/athena/ps_data
		;;
	*)
		rm -rf /tmp/* /tmp/.[^.]* > /dev/null 2>&1
		;;
	esac
	rm -rf /var/athena/tmphomedir/* > /dev/null 2>&1
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
fi

# Restore password and group files
if [ -f /etc/passwd.local ] ; then
	cp -p /etc/passwd.local /etc/ptmp && /bin/mv -f /etc/ptmp /etc/passwd
fi
if [ -f /etc/shadow.local ] ; then
	cp -p /etc/shadow.local /etc/stmp && /bin/mv -f /etc/stmp /etc/shadow
fi
if [ -f /etc/group.local ] ; then
	cp -p /etc/group.local /etc/gtmp && /bin/mv -f /etc/gtmp /etc/group
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
		rm -f /etc/athena/reactivate.local
	fi
fi

if [ -f /usr/athena/bin/access_off ]; then
	/usr/athena/bin/access_off
fi

if [ "$full" = true ]; then
	if [ -f /etc/athena/reactivate.local ]; then
		/etc/athena/reactivate.local
	fi
fi

exit 0
