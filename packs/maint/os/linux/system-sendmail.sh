#!/bin/sh
# $Id: system-sendmail.sh,v 1.1.2.1 2002-12-09 23:30:58 ghudson Exp $
#
# Run the sendmail queue on net-up events.
#

# Read the status file.
. "$1" || exit 1

# Ignore everything but net-up events.
case "$EVENT" in
net-up)
  # Try to flush the sendmail queue in the background.
  (sendmail -q &) > /dev/null 2>&1
  ;;
esac

exit 0
