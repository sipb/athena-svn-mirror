#!/bin/sh
# $Id: track-srvd.sh,v 1.3 2002-04-06 23:16:31 ghudson Exp $

# track-srvd: Track the srvd on a Solaris machine, deciding between
# the regular subscription list (sys_rvd) and the list for machines
# with more space (sys_rvd.big).

# Default to the small list.
big=

set -- `df -k $UPDATE_ROOT/usr | tail -1`
kbytes=$2
avail=$4
part=$6
if [ "$part" = ${UPDATE_ROOT-/} -a "$kbytes" -ge 2831155 ]; then
  # /usr is on the root partition and is at least 3GB (minus 10% for
  # filesystem overhead).  We're all clear to use the big statfile,
  # unless the machine has never used the big stat file before and 
  # doesn't have enough room.  Assume we need 1GB of free space for
  # the big stat file, which (right now) is a tremendous overestimate
  # but should still be true for almost all machines.
  if [ ! -h $UPDATE_ROOT/usr/athena -o "$avail" -ge 1048576 ]; then
    big=".big"
  fi
fi

/srvd/usr/athena/etc/track -q -T "${UPDATE_ROOT-/}" -W /srvd/usr/athena/lib \
  -s stats/sys_rvd$big slists/sys_rvd$big
