#!/bin/sh
# $Id: system-zephyr.sh,v 1.1 2002-12-09 22:56:11 ghudson Exp $
#
# Start zhm at the first net-up event.
#

. /etc/init.d/functions || exit 1
. /etc/athena/rc.conf || exit 1

[ -f /etc/athena/zhm ] || exit 0
[ true = "$ZCLIENT" ] || exit 0

# Read the status file.
. "$1" || exit 1

PATH=/etc/athena:/bin/athena:/user/athena/bin:$PATH

# Ignore everything but net-up events.
case "$EVENT" in
net-up)
  if [ -f /var/athena/zhm.pid ]; then
    kill -HUP `cat /var/athena/zhm.pid` 2> /dev/null && exit 0
  fi

  # On the first net-up event, try to start zhm
  if [ true = "$FIRST_NET_UP" ]; then
    daemon zhm && touch /var/lock/subsys/zhm
  fi
  ;;
esac

exit 0
