#!/bin/sh
# $Id: update_ws.sh,v 1.25 2001-11-28 20:48:11 ghudson Exp $

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

if [ -f /var/athena/clusterinfo.bsh ] ; then
  . /var/athena/clusterinfo.bsh
fi
if [ "${SYSPREFIX+set}" != set ]; then
  SYSPREFIX=/afs/athena.mit.edu/system/rhlinux
fi
exec $SYSPREFIX/control/update "$@"
