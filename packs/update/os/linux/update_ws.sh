#!/bin/sh
# $Id: update_ws.sh,v 1.16 2000-12-30 12:27:31 ghudson Exp $

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

PATH=/usr/athena/bin:/bin/athena:/etc/athena:/bin:/usr/bin:/sbin:/usr/sbin

errorout() {
	echo "$@" >&2
	echo -n "Please contact Athena Cluster Services at x3-1410.  " >&2
	echo "-Athena Operations" >&2
	exit 1
}

# Process options.
auto=
dryrun=false
while getopts a:n opt; do
	case $opt in
	a)
		auto=$OPTARG
		;;
	n)
		dryrun=true
		;;
	\?)
		echo "$0 [-n] [-a reactivate|rc] [version]" >&2
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`

# We accept one optional argument, a version specifier, which may be
# of the form "x.y" (full release number) or "x.y.z" (patch release
# number).  Set versarg to the full release number and pversarg to the
# patch release number, if given.
versarg=$1
case $versarg in
*.*.*)
	pversarg=$versarg
	versarg=`expr $pversarg : '\([0-9]*\.[0-9]*\)'`
	;;
esac

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

# Define a function to set cluster variables for this host for a given
# version.  The "AUTOUPDATE=false" is so getcluster will output the
# current version's cluster variables and NEW_PRODUCTION_RELEASE for a
# new version.  This is a hack; ideally getcluster would always behave
# this way.
getclust() {
	unset NEW_TESTING_RELEASE NEW_PRODUCTION_RELEASE SYSPREFIX SYSCONTROL
	eval `AUTOUPDATE=false getcluster -b -l /etc/athena/cluster.local \
		"$HOST" "$1"`
}

# Fetch the SYSPREFIX and SYSCONTROL cluster variables.
if [ -n "$auto" ]; then
	# Fetch variables for the current production release.
	getclust "$oldvers"
	if [ -n "$NEW_PRODUCTION_RELEASE" ]; then
		getclust "$NEW_PRODUCTION_RELEASE"
	fi
else
	# For manual updates, fetch variables for the release
	# specified on the command line or for the current release if
	# no version was given.
	getclust "${versarg:-$oldvers}"
fi
if [ -z "$SYSPREFIX" -o -z "$SYSCONTROL" ]; then
	errorout "Can't find system cluster information for this machine."
fi

# Change to the system area.
cd "$SYSPREFIX" || errorout "Can't change to system area $SYSPREFIX."

# Decide what the new version and new list file are for this update.
if [ -n "$pversarg" ]; then
	# Find the specified patch version in the control file.
	exec 3< "$SYSCONTROL" || errorout "Can't read `pwd`/$SYSCONTROL."
	unset newlist
	while read version filename throw_away_the_rest <&3; do
		if [ "x$version" = "x$pversarg" ]; then
			newlist=$filename
			break
		fi
	done
	exec 3<&-
	if [ -z "$newlist" ]; then
		echo "Can't find $pversarg in `pwd`/$SYSCONTROL." >&2
		exit 1
	fi
	newvers=$pversarg
else
	# Get the latest version from the control file.
	set -- `tail -1 "$SYSCONTROL"`
	newvers=$1
	newlist=$2
fi

# Define a function to output a message pointing out a new testing release.
new_testing_release_msg() {
	echo "A new Athena release ($NEW_TESTING_RELEASE) is now in testing."
	echo "You are theoretically interested in this phase of testing, but"
	echo "because there may be bugs which would inconvenience your work,"
	echo "you must update to this release manually.  Please contact Athena"
	echo "Cluster Services (x3-1410) if you have not received instructions"
	echo "on how to update."
}

if [ "x$newvers" = "x$oldvers" ]; then
	# There's no new version available.  Print something
	# appropriate and exit.
	if [ -n "$auto" ]; then
		if [ -n "$NEW_TESTING_RELEASE" ]; then
			new_testing_release_msg
		fi
	elif [ -n "$pversarg" ]; then
		echo "You are already at version $pversarg."
	elif [ -n "$NEW_PRODUCTION_RELEASE" -o \
	       -n "$NEW_TESTING_RELEASE" ]; then
		echo "Your workstation is already up to date for this full"
		echo "release.  You must manually specify a newer full release"
		echo "to update beyond this point."
	else
		echo "No new version is available."
	fi
	exit
fi

if [ -n "$auto" -a true != "$AUTOUPDATE" ]; then
	# There's a new version available, but we can't take it yet.
	echo "A new version of Athena software is now available."
	echo "Please contact Athena Cluster Services (x3-1410) to"
	echo "get more information on how to update your workstation"
	echo "yourself, or to schedule us to do it for you."
	echo "    Thank you.  -Athena Operations"
	exit
fi

if [ -n "$auto" ]; then
	# Desynchronize automatic updates by the value of the desync
	# cluster variable or four hours if it isn't set.
	desync -t /var/athena/update.desync ${DESYNC-14400}
	if [ $? -ne 0 ]; then
		exit 0
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

if [ true = "$PUBLIC" -a -d /var/server ]; then
	rm -rf /var/server
fi
if [ -d /var/server ]; then
	MACH=$MACHINE VERS=$newvers mkserv updatetest
	if [ $? -ne 0 ]; then
		errorout "Not all mkserv services available for $newvers."
	fi
fi

# If we're doing a dry run, here's where we get off the train.
if [ true = "$dryrun" ]; then
	echo "Package changes for update from $oldvers to $newvers:"
	rpmupdate -n $publicflag "$oldlist" "$newlist" | sort
	exit 0
fi

if [ reactivate = "$auto" -a -f /var/athena/dm.pid ]; then
	# Tell dm to shut down everything and sleep forever during the update.
	kill -FPE `cat /var/athena/dm.pid`
fi

# Define how to clean up if the update fails.
failupdate() {
	logger -t $HOST -p user.notice "Update ($oldvers -> $newvers) failed"
	echo "Athena Workstation ($hosttype) Version $oldvers `date`" >> \
		/etc/athena/version
	if [ reactivate = "$auto" -a -f /var/athena/dm.pid ]; then
		kill `cat /var/athena/dm.pid`
	fi
	errorout "*** The update has failed ***"
}

# Do the update.
{
	echo "Beginning update from $oldvers to $newvers at `date`."
	echo "Athena Workstation ($hosttype) Version Update `date`" >> \
		/etc/athena/version
	rpmupdate -h $publicflag "$oldlist" "$newlist" || failupdate
	cp "$newlist" "$oldlist" || failupdate
	kudzu -q
	echo "Athena Workstation ($hosttype) Version $newvers `date`" >> \
		/etc/athena/version
	if [ -d /var/server ]; then
		mkserv -v update < /dev/null
	fi
	rm -f /var/athena/rc.conf.sync
	echo "Ending update from $oldvers to $newvers at `date`."
} 2>&1 | tee /var/athena/update.log

if [ -n "$auto" ]; then
	echo "Automatic update done; system will reboot in 15 seconds."
	sync
	sleep 15
	exec reboot
else
	echo "Update complete; please reboot for changes to take effect."
fi
