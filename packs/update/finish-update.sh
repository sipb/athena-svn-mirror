#!/bin/sh
# $Id: finish-update.sh,v 1.21 2004-05-06 20:10:32 rbasch Exp $

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

. $CONFVARS
. $CONFDIR/rc.conf

if [ -s "$MIT_OLD_PACKAGES" ]; then
  echo "Removing old Athena packages."
  for i in `cat "$MIT_OLD_PACKAGES"`; do
    echo "$i"
    pkgrm -a $LIBDIR/admin-update -n -R "${UPDATE_ROOT:-/}" "$i"
  done
  echo "Finished removing old Athena packages."
fi

if [ -s "$MIT_NONCORE_PACKAGES" ]; then
  echo "Installing new Athena non-core packages."
  for i in `cat "$MIT_NONCORE_PACKAGES"`; do
    echo "$i"
    pkgadd -a $LIBDIR/admin-update -n -R "${UPDATE_ROOT:-/}" \
      -d "/srvd/pkg/$newvers" "$i"
  done
  echo "Finished installing Athena packages."
fi

# Run catman to format Athena man pages, and create the windex
# database.  Suppress all output, since catman will complain about
# pages in lockers which cannot be read without tokens (e.g. psutils).
echo "Formatting Athena man pages."
/usr/bin/catman -M $UPDATE_ROOT/usr/athena/man > /dev/null 2>&1

rm -f $UPDATE_ROOT/var/athena/rc.conf.sync

if [ "$OSCHANGES" = true ]; then
  echo "Creating windex databases."
  /usr/bin/catman -w -M $UPDATE_ROOT/usr/openwin/share/man:$UPDATE_ROOT/usr/dt/share/man:$UPDATE_ROOT/usr/share/man
fi

# Remove the version script state files.
rm -f "$CONFCHG" "$CONFVARS" "$AUXDEVS" "$OLDBINS" "$OLDLIBS" "$DEADFILES"
rm -f "$OSCONFCHG" "$PACKAGES" "$PATCHES" "$OLDPKGS" "$OLDPTCHS"
rm -f "$MIT_CORE_PACKAGES" "$MIT_NONCORE_PACKAGES" "$MIT_OLD_PACKAGES"
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
