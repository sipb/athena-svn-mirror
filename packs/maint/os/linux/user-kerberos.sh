#!/bin/sh
# $Id: user-kerberos.sh,v 1.2 2003-02-26 18:44:48 zacheiss Exp $
#
# Manipulate Kerberos tickets as appropriate in response to network events.
#

# Read the status file.
. "$1" || exit 1

case "$EVENT" in
resume|net-up)
  # If hostname changed, then krb524init.
  if [ "x$OLDHOSTNAME" != "x$HOSTNAME" ]; then
    echo "IP Address changed.  Getting new krb4 tickets."
    krb524init
  fi

  ;;
esac

exit 0
