#!/bin/sh
# $Id: sendmail.sh,v 1.1 2003-10-23 22:57:57 ghudson Exp $

# If we don't have tickets, we must do direct delivery and not do
# authentication.
# If we're not using the default MAILRELAY, we can't do authentication.
# Otherwise, we have tickets and are using the ATHENA.MIT.EDU MX record, 
# and can use authentication.

. /etc/athena/rc.conf

/usr/athena/bin/klist -s
if [ $? != 0 ]; then
  DIRECT_DELIVERY=1; export DIRECT_DELIVERY
fi

if [ "$MAILRELAY" != "default" -o -n "$DIRECT_DELIVERY" ]; then
  auth="-U"
fi

exec /usr/lib/sendmail.real $auth "$@"
