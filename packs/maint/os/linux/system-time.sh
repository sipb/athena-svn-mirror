#!/bin/sh
# $Id: system-time.sh,v 1.1 2002-12-09 22:56:11 ghudson Exp $
#
# Reset the system time on network up events.
#

. /etc/athena/rc.conf || exit 1
[ -n "$TIMEHUB" ] || exit 0

# Read the status file.
. "$1" || exit 1

PATH=/etc/athena:/bin/athena:/user/athena/bin:$PATH

# Ignore everything but net-up events.
case "$EVENT" in
net-up)
  # On the first net-up, force-sync the clock.  On all others,
  # only resync if we're more than 240 seconds off.
  if [ true = "$FIRST_NET_UP" ]; then
    gettime -s "$TIMEHUB" > /dev/null
  else
    gettime -s -g 240 "$TIMEHUB" > /dev/null
  fi
  ;;
esac

exit 0
