#!/bin/sh
# $Id: finish-update.sh,v 1.19 2003-04-10 05:59:55 ghudson Exp $

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

echo "Creating config files for Athena software"
sh /srvd/install/athchanges

# Remove the version script state files.
rm -f "$CONFCHG" "$CONFVARS" "$AUXDEVS" "$OLDBINS" "$OLDLIBS" "$DEADFILES"
rm -f "$CONFIGVERS" "$PACKAGES" "$PATCHES"
if [ -n "$PACKAGES" ]; then
	rm -f "$PACKAGES".*
fi

# Convert old attachtab files.
if [ -f /var/athena/attachtab.old ]; then
	echo "Converting old attachtab to liblocker format."
	/srvd/usr/athena/etc/atconvert /var/athena/attachtab.old \
		&& rm -f /var/athena/attachtab.old
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
rm -rf /var/athena/update.running
