#!/bin/sh
# Script to bounce the packs on an Athena workstation
#
# $Id: reactivate.sh,v 1.66 2001-09-16 17:25:38 ghudson Exp $

# Ignore various terminating signals.
trap "" HUP INT QUIT PIPE ALRM TERM USR1 USR2

PATH=/bin:/etc/athena:/bin/athena:/usr/bin:/usr/sbin:/usr/ucb:/usr/bsd:/sbin; export PATH
HOSTTYPE=`/bin/athena/machtype`; export HOSTTYPE

pidfile=/var/athena/reactivate.pid
countfile=/var/athena/reactivate.count
nologin=/etc/nologin
made_nologin=false
afsconfig=/afs/athena.mit.edu/system/config/afs

umask 22
. /etc/athena/rc.conf

# Quit now if in the middle of an update.
THISVERS=`awk '{a=$5} END{print a}' /etc/athena/version`
case "$THISVERS" in
[0-9]*|Layered)
	# Not in an update, OK.
	;;
*)
	# In an update, quit now.
	echo "reactivate: This workstation is in the middle of an update."
	exit 1
	;;
esac

if [ "$1" = -prelogin ]; then
	if [ "$PUBLIC" = "false" ]; then
		exit 0;
	fi
	echo "Cleaning up..." >> /dev/console
	full=false
else
	full=true
fi

# Quit now if another reactivate process is running.
if [ -s $pidfile ]; then
	pid=`cat $pidfile 2>/dev/null`
	if [ -n "$pid" -a "$pid" -ne 0 ]; then
		kill -0 $pid 2>/dev/null
		if [ $? -eq 0 ]; then
			echo "Another reactivate process is running ($pid)."
			exit 0
		fi
	fi
fi

echo $$ > $pidfile

# Define a function to clean up at exit.
# We want to ensure that we don't leave logins disabled.
# This function also removes our pid file.
# (Note that terminating signals are ignored, above).
cleanexit()
{
	if [ true = "${made_nologin}" ]; then
		rm -f $nologin
	fi
	rm -f $pidfile
}

trap cleanexit EXIT

# See if anyone is logged in.  We check for stale utmp entries, by
# doing a kill -0 on the session leader's pid.
# The Linux who does not give the pid, so we must use ps to figure
# it out.
if [ "$full" = true ]; then
	if [ linux = "$HOSTTYPE" ]; then
		pids=
		# Use w instead of who, since it ignores stale utmp entries.
		for tty in `w -h -s | awk '{ print $2; }'` ; do
			pids="$pids `ps --no-heading -j -t $tty 2>/dev/null | \
				awk '($1 == $3) { print $1; }'`"
		done
	else
		pids=`who -u | awk '{ print $7; }'`
	fi

	# If any session leader pid is current, quit now.  Ignore dm
	# (which is the session leader on the console tty), in case of
	# a stale utmp entry from a console login.
	dmpid=`cat /var/athena/dm.pid 2>/dev/null`
	for pid in $pids ; do
		if [ "$pid" != "$dmpid" ]; then
			kill -0 $pid 2>/dev/null
			if [ $? -eq 0 ]; then
				rm -f $countfile
				exit 0
			fi
		fi
	done

	# Check for valid Athena session records; these get created for
	# remote shells, etc., which may not have an associated utmp entry.
	# Quit if any are found.

	# We need to use nawk on Solaris in parsing the sessions file below.
	case "$HOSTTYPE" in
	sun4)
		awk=nawk
		;;
	*)
		awk=awk
		;;
	esac

	for i in /var/athena/sessions/* ; do
		if [ -s $i ]; then
			for pid in `					\
			  $awk -F : '					\
			    FNR == 5					\
			    {						\
				for (i = 1; i <= NF; i++)		\
				    if (int($i) != 0)			\
					print $i;			\
			    }' $i` ; do
				kill -0 $pid 2>/dev/null
				if [ $? -eq 0 ]; then
					rm -f $countfile
					exit 0
				fi
			done
		fi
	done

	local-menus
fi

# There are no current logins or sessions, so proceed.  We disable
# logins for the duration, by creating /etc/nologin, unless it
# already exists.
if [ ! -f $nologin ]; then
	made_nologin=true
	echo "Workstation is reactivating." > $nologin
fi

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

if [ -f /var/athena/clusterinfo.bsh ] ; then
	. /var/athena/clusterinfo.bsh
fi

# Determine where the config files live
if [ "$HOSTTYPE" = linux -a -n "$SYSPREFIX" ]; then
	config=$SYSPREFIX/config/$THISVERS
else
	config=/srvd
fi

# We don't want to detach all filesystems on every invocation, so
# we keep a count file, and only detach all every tenth invocation,
# or when the -detach option is specified.
count=`cat $countfile 2>/dev/null`
if [ -z "$count" ]; then
	count=0
fi
if [ "$1" = -detach -o `expr $count % 10` -eq 0 ]; then
	dflags=""
else
	dflags="-clean"
fi

if [ ! -t 0 ]; then
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

# Clean up occasional leavings of emacs and esd.
rm -rf /var/tmp/!!!SuperLock!!! /tmp/.esd

# Remove utmp and wtmp so Solaris doesn't complain.
if [ sun4 = "$HOSTTYPE" ]; then
	rm -rf /var/adm/utmp /var/adm/wtmp
fi

# Clean up socket files left by sawfish.
rm -rf /tmp/.sawfish-*

if [ "$full" = true ]; then
	# Clean temporary areas (including temporary home directories)
	if [ "$PUBLIC" = true ]; then
		if [ sun4 = "$HOSTTYPE" -a -f /tmp/ps_data ]; then
			cp -p /tmp/ps_data /var/athena/ps_data
			nuke /tmp > /dev/null 2>&1
			cp -p /var/athena/ps_data /tmp/ps_data
			rm -f /var/athena/ps_data
		else
			nuke /tmp > /dev/null 2>&1
		fi
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
		syncupdate -c /etc/group.local.new $config/etc/group \
			/etc/group.local
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
	# If the encrypt file doesn't exist, disable AFS encryption.
	# Only do this on Linux for now, since other platforms aren't
	# running a client that supports encryption.
	if [ linux = "$HOSTTYPE" ]; then
		if  [ -f $afsconfig/encrypt ]; then
			/bin/athena/fs setcrypt on
		else
			/bin/athena/fs setcrypt off
		fi
		# Work around an OpenAFS bug: if a Linux client boots
		# while its root.afs is unavailable, it will get into
		# a broken state that can only be fixed by rebooting.
		# Check for this state and reboot here.
		if [ "$PUBLIC" = "true" -a "$AFSCLIENT" = "true" ]; then
			module=`lsmod | grep afs`
			mounted=`mount | grep afs`
			if [ -n "$module" -a -z "$mounted" ]; then
				logger -p user.notice "AFS loaded in kernel but not mounted.  Rebooting."
				reboot
			fi
		fi
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
			case "$HOSTTYPE" in
			sun4)
				/srvd/usr/athena/lib/update/track-srvd
				;;
			*)
				/usr/athena/etc/track -q
				;;
			esac
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
	# Update our invocation count.
	echo `expr $count + 1` > $countfile
fi

exit 0
