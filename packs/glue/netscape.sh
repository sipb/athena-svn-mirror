#!/bin/sh
# $Id: netscape.sh,v 1.2 2000-07-29 14:18:23 ghudson Exp $

localscript=/var/athena/infoagents/arch/share/bin/netscape.adjusted
if [ -x $localscript ]; then
  /bin/athena/attach -nq infoagents
  exec $localscript "$@"
else
  exec /bin/athena/attachandrun infoagents netscape "$0" "$@"
fi
