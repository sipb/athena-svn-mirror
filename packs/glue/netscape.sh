#!/bin/sh
# $Id: netscape.sh,v 1.1.2.1 2000-08-01 19:08:02 ghudson Exp $

localscript=/var/athena/infoagents/arch/share/bin/netscape.adjusted
if [ -x $localscript ]; then
  /bin/athena/attach -nq infoagents
  exec $localscript "$@"
else
  exec /bin/athena/attachandrun infoagents netscape "$0" "$@"
fi
