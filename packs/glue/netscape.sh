#!/bin/sh
# $Id: netscape.sh,v 1.1 2000-06-08 18:49:33 ghudson Exp $

localscript=/var/athena/infoagents/arch/share/bin/netscape.adjusted
if [ -x $localscript ]; then
  exec $localscript "$@"
else
  exec /bin/athena/attachandrun infoagents netscape "$0" "$@"
fi
