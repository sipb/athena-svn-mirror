#!/bin/sh
# $Id: update_ws.sh,v 1.8 2000-03-04 17:39:36 ghudson Exp $

# Copyright 2000 by the Massachusetts Institute of Technology.
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

# update_ws
#
# Check that an update is needed, and make sure the conditions necessary
# for a successful update are met. Then prepare the machine for update,
# and run do-update.

PATH=/etc/athena:/bin:/usr/bin:/sbin:/usr/sbin

errorout() {
	echo "$@" 1>&2
	echo -n "Please contact Athena Cluster Services at x3-1410.  " 1>&2
	echo "-Athena Operations" 1>&2
	exit 1
}

# Process options.
auto=false
dryrun=false
while getopts an opt; do
	case $opt in
	a)
		auto=true
		;;
	n)
		dryrun=true
		;;
	\?)
		echo "$0 [-a] [reactivate|rc]" 1>&2
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`
why="$1"

. /etc/athena/rc.conf
hosttype=`/bin/athena/machtype`

# Get the current workstation version and see if we're already in the
# middle of an update.
oldvers=`awk '{ a = $5; } END { print a; }' /etc/athena/version`
case "$oldvers" in
[0-9]*)
	# Looks okay.
	;;
Layered)
	errorout "You cannot take an update on a Layered machine."
	;;
*)
	# Doesn't look okay.  Complain and exit.
	if [ ! -f /var/athena/update.check ]; then
		logger -t $HOST -p user.notice at revision $oldvers
		touch /var/athena/update.check
	fi

	errorout "This system is in the middle of an update."
	;;
esac

# Get and read cluster information.  This will set NEW_TESTING_RELEASE
# and/or NEW_PRODUCTION_RELEASE if there are new full releases
# available, and will also set sysprefix and syscontrol to the
# correct paths for the current full release.
unset NEW_TESTING_RELEASE NEW_PRODUCTION_RELEASE SYSPREFIX SYSCONTROL
/etc/athena/save_cluster_info
if [ -f /var/athena/clusterinfo.bsh ]; then
	. /var/athena/clusterinfo.bsh
fi
if [ -z "$SYSPREFIX" -o -z "$SYSCONTROL" ]; then
	errorout "Can't find system cluster information for this machine."
fi

# Change to the system area.
cd "$SYSPREFIX" || errorout "Can't change to system area $SYSPREFIX."

# Get the latest version from the control file.
set -- `tail -1 $SYSCONTROL`
newvers=$1
newlist=$2

if [ "x$newvers" = "x$oldvers" ]; then
	# There's no new version available.  Print something and exit.
	# XXX This is where we should deal with NEW_TESTING_RELEASE and
	# NEW_PRODUCTION_RELEASE, but right now we don't have a manual
	# way to update to new full releases so we don't have much to
	# say about them.
	if [ false = "$auto" ]; then
		echo "No new version is available."
	fi
	exit
fi

if [ true = "$auto" -a true != "$AUTOUPDATE" ]; then
	# There's a new version available, but we can't take it yet.
	echo "A new version of Athena software is now available."
	echo "Please contact Athena Cluster Services (x3-1410) to"
	echo "get more information on how to update your workstation"
	echo "yourself, or to schedule us to do it for you."
	echo "    Thank you.  -Athena Operations"
	exit
fi

if [ true = "$auto" -a reactivate = "$why" ]; then
	# Tell dm to shut down everything and sleep forever during the update.
	if [ -f /var/athena/dm.pid ]; then
		kill -FPE `cat /var/athena/dm.pid`
	fi
fi

# Translate public status into command-line flag and old list filename.
# rpmupdate does not use the information from the old list for public
# updates.
if [ true = "$PUBLIC" ]; then
	publicflag=-p
else
	publicflag=
fi

oldlist=/var/athena/release-rpms
if [ ! -r "$oldlist" ]; then
	errorout "Cannot read old release list $oldlist."
fi

# If we're doing a dry run, here's where we get off the train.
if [ true = "$dryrun" ]; then
	echo "Package changes for update from $oldvers to $newvers:"
	rpmupdate -n $publicflag "$oldlist" "$newlist" | sort
	exit 0
fi

# Define how to clean up if the update fails.
failupdate() {
	logger -t $HOST -p user.notice "Update ($oldvers -> $newvers) failed"
	echo "Athena Workstation ($hosttype) Version $oldvers `date`" >> \
		/etc/athena/version
	errorout "*** The update has failed ***"
}

# Do the update.
{
	echo "Beginning update from $oldvers to $newvers at `date`."
	echo "Athena Workstation ($hosttype) Version Update `date`" >> \
		/etc/athena/version
	rpmupdate $publicflag "$oldlist" "$newlist" || failupdate
	cp "$newlist" "$oldlist" || failupdate
	echo "Athena Workstation ($hosttype) Version $newvers `date`" >> \
		/etc/athena/version
	echo "Ending update from $oldvers to $newvers at `date`."
} 2>&1 | tee /var/athena/update.log

if [ true = "$auto" ]; then
	echo "Automatic update done; system will reboot in 15 seconds."
	sync
	sleep 15
	exec reboot
else
	echo "Update complete; please reboot for changes to take effect."
fi
