#!/bin/sh
# $Id: system-afs.sh,v 1.2 2006-03-13 21:26:01 ghudson Exp $
#
# Check the AFS servers at net-up events.
#

# Read the status file.
. "$1" || exit 1

PATH=/etc/athena:/bin/athena:/user/athena/bin:$PATH

# Ignore everything but net-up events.
case "$EVENT" in
net-up)
  if mount | grep -q AFS; then
    fs checks -all -fast > /dev/null 2>&1
  fi

  # On the first net-up event, try to start AFS
  if [ true = "$FIRST_NET_UP" ]; then
    sh /etc/init.d/openafs netevent-start
  fi
  ;;
esac

exit 0
