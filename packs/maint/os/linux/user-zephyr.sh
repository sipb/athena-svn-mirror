#!/bin/sh
# $Id: user-zephyr.sh,v 1.1 2002-12-09 22:56:14 ghudson Exp $
#
# Refresh zephyr subscriptions on network up events.
#

# Read the status file.
. "$1" || exit 1

case "$EVENT" in
resume|net-up)
  klist -s || exit 0
  if [ ! -z "$WGFILE" -a -f "$WGFILE" ]; then
    echo "Reloading zephyr subscriptions"
    zctl load
  else
    echo "Zephyr not running, cannot reload subscriptions."
  fi
  ;;
esac

exit 0
