#!/bin/sh
# $Id: finish-update.sh,v 1.14 1999-04-01 21:08:36 ghudson Exp $

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

echo "Starting the second stage of the update at `date`."
. /srvd/usr/athena/lib/update/update-environment

# We get one argument, the new workstation version we're updating to.
newvers="$1"

. $CONFDIR/rc.conf

# Do auxiliary device installs.
if [ -s "$AUXDEVS" ]; then
	drvrs=`cat "$AUXDEVS"`
	for i in $drvrs; do
		/srvd/install/aux.devs/$i
	done
fi

# For a public workstation, remove old and new copies of
# config files left behind by inst (Irix only).
# Currently, these are hard-coded in the version script.
if [ "$PUBLIC" = "true" -a -s "$CONFIGVERS" ]; then
	for i in `cat "$CONFIGVERS"` ; do
		rm -f $i
	done
fi

# Remove the version script state files.
rm -f "$CONFCHG" "$CONFVARS" "$AUXDEVS" "$OLDBINS" "$OLDLIBS" "$DEADFILES"
rm -f "$CONFIGVERS"
if [ -n "$LOCALPACKAGES" ]; then
	rm -f "$LOCALPACKAGES".*
fi
if [ -n "$LINKPACKAGES" ]; then
	rm -f "$LINKPACKAGES".*
fi

# Convert old attachtab files.
if [ -f /var/athena/attachtab ]; then
	echo "Updating attachtab to liblocker format."
	mv /var/athena/attachtab /var/athena/attachtab.old
	mkdir /var/athena/attachtab
	mkdir /var/athena/attachtab/locker
	mkdir /var/athena/attachtab/mountpoint
	mkdir /var/athena/attachtab/directory
	/srvd/usr/athena/etc/atconvert /var/athena/attachtab.old
fi

echo "Updating version"
echo "Athena Workstation ($HOSTTYPE) Version $newvers `date`" >> \
	$CONFDIR/version

# Re-customize the workstation
if [ "$PUBLIC" = "true" ]; then
	rm -rf "$SERVERDIR"
fi

if [ -d "$SERVERDIR" ]; then
	echo "Running mkserv."
	/srvd/usr/athena/bin/mkserv -v update < /dev/null
fi
echo "Finished with update at `date`."
