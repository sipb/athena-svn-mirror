#!/bin/sh

# $Id: xkill-mozilla.sh,v 1.1 2004-06-07 19:10:49 rbasch Exp $

# Kill all of the Mozilla instances running on this X display.

moz_libdir=/usr/athena/lib/mozilla

ping_mozilla () {
  LD_LIBRARY_PATH=$moz_libdir${LD_LIBRARY_PATH:+":$LD_LIBRARY_PATH"} \
    $moz_libdir/mozilla-xremote-client "ping()" > /dev/null 2>&1
  return $?
}

killed=
timeout=3
i=0
while ping_mozilla ; do
  # Find all windows which plausibly belong to Mozilla.
  for win in `xwininfo -root -tree | awk '/Mozilla/ { print $1; }'` ; do
    # Skip any window we have already tried to kill.
    case $killed in
    *" $win "*)
      continue
      ;;
    esac
    # See if the window has the _MOZILLA_VERSION property set.
    moz_version="`xprop -id $win _MOZILLA_VERSION 2>/dev/null | \
      sed -n -e 's/.*"\(.*\)"/\1/p'`"
    if [ -n "$moz_version" ]; then 
      # Found one.  Kill it, add it to the list of killed windows, and
      # go back to the top of the outer loop to check against the new
      # set of windows.
      xkill -id $win > /dev/null 2>&1
      killed=" $killed $win "
      continue 2
    fi
  done
  # If we get here, the ping found a running mozilla, but we were unable
  # to find a window which had not already been killed.  Wait one second
  # and try again, up to our timeout limit.
  if [ `expr $i \>= $timeout` -eq 1 ]; then
    echo "Unable to kill Mozilla" 1>&2
    exit 1
  fi
  sleep 1
  i=`expr $i + 1`
done

exit 0
