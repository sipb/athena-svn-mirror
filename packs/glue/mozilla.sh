#!/bin/sh
# $Id: mozilla.sh,v 1.1.2.1 2002-09-10 20:24:10 ghudson Exp $

localscript=/var/athena/infoagents/arch/share/bin/mozilla.adjusted
if [ -x $localscript ]; then
  locker=`/bin/athena/attach -np infoagents`
  if [ -f "$locker/.nolocal" ]; then
    exec /bin/athena/attachandrun infoagents mozilla "$0" "$@"
  else
    exec $localscript "$@"
  fi
else
  exec /bin/athena/attachandrun infoagents mozilla "$0" "$@"
fi
