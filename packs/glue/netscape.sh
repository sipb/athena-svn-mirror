#!/bin/sh
# $Id: netscape.sh,v 1.1.2.2 2000-09-23 19:32:39 ghudson Exp $

localscript=/var/athena/infoagents/arch/share/bin/netscape.adjusted
if [ -x $localscript ]; then
  locker=`/bin/athena/attach -np infoagents`
  if [ -f "$locker/.nolocal" ]; then
    exec /bin/athena/attachandrun infoagents netscape "$0" "$@"
  else
    exec $localscript "$@"
  fi
else
  exec /bin/athena/attachandrun infoagents netscape "$0" "$@"
fi
