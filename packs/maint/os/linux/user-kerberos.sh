#!/bin/sh
# $Id: user-kerberos.sh,v 1.3 2003-06-10 15:14:08 ghudson Exp $
#
# Manipulate Kerberos tickets as appropriate in response to network events.
#

# Read the status file.
. "$1" || exit 1

case "$EVENT" in
net-up)
  # If this is the only interface and the IP Address changed
  # then get new v4 tickets
  if [ $NETDEVCOUNT -eq 1 -a "x$OLDIPADDR" != "x$IPADDR" ]; then
    echo "IP Address changed.  Getting new krb4 tickets."
    krb524init
  fi

  ;;
esac

exit 0
