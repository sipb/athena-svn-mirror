#!/bin/sh
# $Id: system-afs.sh,v 1.1 2002-12-09 22:56:09 ghudson Exp $
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
  ;;
esac

exit 0
