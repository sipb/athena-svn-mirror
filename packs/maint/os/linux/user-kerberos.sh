#!/bin/sh
# $Id: user-kerberos.sh,v 1.3.4.1 2005-01-04 18:28:11 ghudson Exp $
#
# Manipulate Kerberos tickets as appropriate in response to network events.
#

# Read the status file.
. "$1" || exit 1

case "$EVENT" in
net-up)
  # If this is the only interface and the IP Address changed
  # then get new v4 tickets
  if [ "$NETDEVCOUNT" -eq 1 -a "x$OLDIPADDR" != "x$IPADDR" ]; then
    echo "IP Address changed.  Getting new krb4 tickets."
    krb524init
  fi

  ;;
esac

exit 0
