#!/bin/sh
# $Id: user-kerberos.sh,v 1.1 2002-12-09 22:56:12 ghudson Exp $
#
# Manipulate Kerberos tickets as appropriate in response to network events.
#

# Read the status file.
. "$1" || exit 1

case "$EVENT" in
resume|net-up)
  # If hostname changed, then kdestroy.
  if [ "x$OLDHOSTNAME" != "x$HOSTNAME" ]; then
    echo "IP Address changed.  Tickets destroyed.."
    kdestroy -q
  fi

  klist -s
  if [ $? -ne 0 ]; then
    if [ -n "$DISPLAY" ]; then
      grenew
    else
      echo "You need new tickets. Get them now by typing \"renew\"."
      echo "You have 60 seconds...."
      sleep 60
    fi
  fi
  ;;
esac

exit 0
