#!/bin/sh
# $Id: system-bind.sh,v 1.1 2003-07-02 20:10:10 ghudson Exp $
#
# HUP the named daemon to refresh the IP address.
#

# Read the status file.
. "$1" || exit 1

PATH=/etc/athena:/bin/athena:/user/athena/bin:$PATH

# Ignore everything but net-up events.
case "$EVENT" in
net-up)
  # if we got a new IP address on the primary (first) interface
  # then HUP the named process to make sure we're listening properly.
  if [ -f /var/athena/named.pid -a $NETDEVCOUNT -eq 1 -a \
      "x$OLDIPADDR" != "x$IPADDR" ]; then
    kill -HUP `cat /var/athena/named.pid` 2> /dev/null && exit 0
  fi
  ;;
esac

exit 0
