#!/bin/sh
# $Id: update_ws.sh,v 1.43 2000-03-13 18:09:26 rbasch Exp $

# Copyright 1996 by the Massachusetts Institute of Technology.
#
# Permission to use, copy, modify, and distribute this
# software and its documentation for any purpose and without
# fee is hereby granted, provided that the above copyright
# notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting
# documentation, and that the name of M.I.T. not be used in
# advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.
# M.I.T. makes no representations about the suitability of
# this software for any purpose.  It is provided "as is"
# without express or implied warranty.

# update_ws (also known as auto_update)
#
# Check that an update is needed, and make sure the conditions necessary
# for a successful update are met. Then prepare the machine for update,
# and run do-update.

# "tee" doesn't work reliably across OS versions (since it's not local on
# Solaris), so emulate it in the shell.
shelltee() {
	exec 3>$1
	while IFS="" read line; do
		echo "$line"
		echo "$line" 1>&3
	done
}

trap "" 1 15

export CONFDIR LIBDIR PATH HOSTTYPE
CONFDIR=/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
PATH=/bin:/etc:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/etc/athena:/bin/athena:/os/usr/bin:/usr/athena/sbin:/os/usr/ucb:/os/usr/bsd:$LIBDIR
HOSTTYPE=`/bin/athena/machtype`

case "$0" in
*auto_update)
	method=Auto
	;;
*)
	method=Manual
	;;
esac

# The -a option specifies that the update is automatic (run by the
# boot script or reactivate).  The -r option specifies that the update
# is remote and that we shouldn't give the user a shell after the
# reboot.
while getopts ar opt; do
	case "$opt" in
	a)
		method=Auto
		;;
	r)
		method=Remote
		;;
	\?)
		echo "$0 [-ar] [reactivate|rc]" 1>&2
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`
why="$1"

case `id` in
"uid=0("*)
	;;
*)
	echo "You are not root.  This update script must be run as root."
	exit 1
	;;
esac

# If /srvd is not mounted, quit.
if [ ! -d /srvd/bin ]; then
	exit 1
fi

if [ ! -f "$CONFDIR/version" ]; then
	echo "Athena Update (???) Version 0.0A Mon Jan 1 00:00:00 EDT 0000" \
		> "$CONFDIR/version"
fi

newvers=`awk '{a=$5}; END{print a}' /srvd/.rvdinfo`
version=`awk '{a=$5}; END{print a}' "$CONFDIR/version"`

# A temporary backward compatibility hack, necessary as long as there are
# 7.7 and 8.0 machines upgrading to the new release.
case "$version" in
[0-9].[0-9][A-Z])
	version=`echo $version | awk '{ print substr($1, 1, 3) "." \
		index("ABCDEFGHIJKLMNOPQRSTUVWXYZ", substr($1, 4, 1)) - 1; }'`
	;;
esac

if [ -f "$CONFDIR/rc.conf" ]; then
	. "$CONFDIR/rc.conf"
else
	export PUBLIC AUTOUPDATE HOST
	PUBLIC=true
	AUTOUPDATE=true
	HOST=`hostname`
fi

# Make sure /var/athena exists (it was introduced in 8.1.0) so we have a
# place to put the temporary clusterinfo file and the desync state file
# and the update log.
if [ ! -d /var/athena ]; then
	mkdir -m 755 /var/athena
fi

# Get and read cluster information, to set one or both of
# NEW_TESTING_RELEASE and NEW_PRODUCTION_RELEASE if there are new
# releases available.
/etc/athena/save_cluster_info
if [ -f /var/athena/clusterinfo.bsh ]; then
	. /var/athena/clusterinfo.bsh
fi

# Check if we're already in the middle of an update.
case "$version" in
[0-9]*)
	# If this field starts with a digit, we're running a proper
	# release. Otherwise...
	;;
*)
	if [ ! -f /var/tmp/update.check ]; then
		logger -t $HOST -p user.notice at revision $version
		touch /var/tmp/update.check
	fi

	echo "This system is in the middle of an update.  Please contact"
	echo "Athena Cluster Services at x3-1410. Thank you. -Athena Operations"
	exit 1
	;;
esac

# Find out if the version in /srvd/.rvdinfo is newer than
# /etc/athena/version.  Distinguish between major, minor, and patch
# releases so that we can desynchronize patch releases.
packsnewer=`echo "$newvers $version" | awk '{
	split($1, v1, ".");
	split($2, v2, ".");
	if (v1[1] + 0 > v2[1] + 0)
		print "major";
	else if (v1[1] + 0 == v2[1] + 0 && v1[2] + 0 > v2[2] + 0)
		print "minor";
	else if (v1[1] == v2[1] && v1[2] == v2[2] && v1[3] + 0 > v2[3] + 0)
		print "patch"; }'`

# If the packs aren't any newer, print an appropriate message and exit.
if [ -z "$packsnewer" ]; then
	if [ "$method" != Auto ]; then
		# User ran update_ws; display something appropriate.
		if [ -n "$NEW_PRODUCTION_RELEASE" -o \
		     -n "$NEW_TESTING_RELEASE" ]; then
			echo "Your workstation software already matches the"
			echo "version on the system packs.  You must manually"
			echo "attach a newer version of the system packs to"
			echo "update beyond this point."
		else
			echo "It appears you already have this update."
		fi
	else
		# System ran auto_update; point out new releases if available.
		if [ -n "$NEW_PRODUCTION_RELEASE" ]; then
			/bin/cat <<EOF

A new Athena release ($NEW_PRODUCTION_RELEASE) is available.  Since it may be
incompatible with your workstation software, your workstation
is still using the old system packs.  Please contact Athena
Cluster Services (x3-1410) to have your workstation updated.
EOF
		fi
		if [ -n "$NEW_TESTING_RELEASE" ]; then
			/bin/cat << EOF

A new Athena release ($NEW_TESTING_RELEASE) is now in testing.  You are
theoretically interested in this phase of testing, but
because there may be bugs which would inconvenience
your work, you must update to this release manually.
Please contact Athena Cluster Services (x3-1410) if 
you have not received instructions on how to do so.
EOF
		fi
	fi
	exit 0
fi

# The packs are newer, but if we were run as auto_update, we don't want to do
# an update unless the machine is autoupdate (or public).
if [ "$method" = Auto -a "$AUTOUPDATE" != true -a "$PUBLIC" != true ]; then
	cat <<EOF

	A new version of Athena software is now available.
	Please contact Athena Cluster Services (x3-1410) to 
	get more information on how to update your workstation 
	yourself, or to schedule us to do it for you. 
	    Thank you.  -Athena Operations
EOF
	if [ ! -f /var/tmp/update.check ]; then
		logger -t $HOST -p user.notice at revision $version
		cp /dev/null /var/tmp/update.check
	fi

	exit 1
fi

if [ "$method" = Auto -a "$packsnewer" = patch ]; then
	# There is a patch release available and we want to take the
	# update, but not necessarily right now.  Use desync to
	# stagger the update over a four-hour period.  (Use the
	# version from /srvd for now to make sure the -t option works,
	# since that option was not introduced until 8.1.)  Note
	# that we only do desynchronization here for patch releases.
	# Desynchronization for major or minor releases is handled in
	# getcluster, since we don't want the workstation to run with
	# a new, possibly incompatible version of the packs.

	/srvd/etc/athena/desync -t /var/athena/update.desync 14400
	if [ $? -ne 0 ]; then
		exit 0
	fi
fi

# The 8.1 -> 8.2 update consumes about 1.7MB on the /usr partition and
# about 3.9MB on the root partition.  Since the smallest-sized Solaris
# partitions only have 5.7MB free on /usr and 8.1MB free on the root
# filesystem under 8.1, it's important to make sure that neither
# filesystem fills up.  Some day the version scripts should take care
# of setting the numbers for this check, but that requires adding
# multiple phases to the version scripts.
case "$HOSTTYPE,$version" in
sun4,8.[01].*|sun4,7.*)
	if [ "`df -k /usr | awk '/\/usr$/ { print $4; }'`" -lt 3072 ]; then
		echo "/usr partition low on space (less than 3MB); not"
		echo "performing update.  Please reinstall or clean local"
		echo "files off /usr partition."
		exit 1
	fi
	if [ "`df -k / | awk '/\/$/ { print $4; }'`" -lt 5120 ]; then
		echo "Root partition low on space (less than 5MB); not"
		echo "performing update.  Please reinstall or clean local"
		echo "files off root partition."
		exit 1
	fi
	;;
esac

# Ensure that we have enough disk space on the IRIX root partition
# for an OS upgrade.
# We also require a minimum of 64MB of memory on all SGI's.
case "$HOSTTYPE" in
sgi)
	case "`uname -r`" in
	6.2)
		rootneeded=130
		;;
	6.3)
		rootneeded=70
		;;
	6.5)
		case "`uname -R | awk '{ print $2; }'`" in
		6.5.3m)
			rootneeded=30
			;;
		*)
			rootneeded=0
			;;
		esac
		;;
	*)
		rootneeded=0
		;;
	esac

	rootfree=`df -k / | awk '$NF == "/" { print int($5 / 1024); }'`
	if [ "$rootfree" -lt "$rootneeded" ]; then
		echo "Root partition low on space (less than ${rootneeded}MB);"
		echo "not performing update.  Please reinstall or"
		echo "clean local files off root partition."
		exit 1
	fi

	if [ "`hinv -t memory | awk '{ print $4; }'`" -lt 64 ]; then
		echo "Insufficient memory (less than 64MB); not"
		echo "performing update.  Please add more memory"
		echo "or reinstall."
		exit 1
	fi
	;;
esac

# If this is a private workstation, make sure we can recreate the mkserv
# state of the machine after the update.
if [ -d /var/server ] ; then
	/srvd/usr/athena/bin/mkserv updatetest
	if [ $? -ne 0 ]; then
		echo "mkserv services cannot be found for all services."
		echo "Update cannot be performed."
		exit 1
	fi
fi

# Tell dm to shut down everything and sleep forever during the update.
if [ "$method" = Auto -a "$why" = reactivate ]; then
	if [ -f /var/athena/dm.pid ]; then
		kill -FPE `cat /var/athena/dm.pid`
	fi
	# 8.0 and prior machines still have /etc/athena/dm.pid.
	if [ -f /etc/athena/dm.pid ]; then
		kill -FPE `cat /etc/athena/dm.pid`
	fi

	if [ -f /etc/init.d/axdm ]; then
		/etc/init.d/axdm stop
 	fi

	sleep 2
	if [ "$HOSTTYPE" = sgi ]; then
		exec 1>/dev/tport 2>&1
	fi
fi

# Everything is all set; do the actual update.
rm -f /var/athena/update.log
if [ "$method" = Auto ]; then
	echo
	echo THIS WORKSTATION IS ABOUT TO UNDERGO AN AUTOMATIC SOFTWARE UPDATE.
	echo THIS PROCEDURE MAY TAKE SOME TIME.
	echo
	echo PLEASE DO NOT DISTURB IT WHILE THIS IS IN PROGRESS.
	echo
	exec sh "$LIBDIR/do-update" "$method" "$version" "$newvers" \
		< /dev/null 2>&1 | shelltee /var/athena/update.log
else
	exec sh "$LIBDIR/do-update" "$method" "$version" "$newvers" \
		2>&1 | shelltee /var/athena/update.log
fi
