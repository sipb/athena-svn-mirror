#!/bin/sh
# $Id: sendmail.sh,v 1.3 2003-11-13 01:50:30 ghudson Exp $

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

if [ "$MAILRELAY" != "default" -o ! -f /etc/athena/sendmail.conf \
     -o -n "$DIRECT_DELIVERY" ]; then
  flags="-U"
else
  flags="-P 587"
fi

exec /usr/sbin/sendmail.real $flags "$@"
