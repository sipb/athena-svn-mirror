#!/bin/sh
# $Id: finish-update.sh,v 1.1 1996-12-27 22:03:48 ghudson Exp $

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

export PATH
CONFDIR=/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
SERVERDIR=/var/server
PATH=/os/bin:/os/etc:/srvd/etc/athena:/srvd/bin/athena:/os/usr/bin:/srvd/usr/athena/etc:/os/usr/ucb:/os/usr/bsd:$LIBDIR:/bin:/etc:/usr/bin:/usr/ucb:/usr/bsd
HOSTTYPE=`/srvd/bin/athena/machtype`

. $CONFDIR/rc.conf

case "$HOSTTYPE" in
sgi)
	echo "Un-suppressing network daemons for next reboot"
	chkconfig -f suppress-network-daemons off
	;;
esac

echo "Updating version"
NEWVERS=`awk '{a=$6}; END{print a}' $CONFDIR/version`
echo "Athena Workstation ($HOSTTYPE) Version $NEWVERS `date`" >> \
	${CONFDIR}/version

# Re-customize the workstation
if [ "$PUBLIC" = "true" ]; then
	rm -rf "$SERVERDIR"
fi

if [ -d "$SERVERDIR" ]; then
	echo "Running mkserv."
	/srvd/usr/athena/bin/mkserv -v update
fi
