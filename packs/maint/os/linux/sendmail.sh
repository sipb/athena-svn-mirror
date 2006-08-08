#!/bin/sh
# $Id: sendmail.sh,v 1.4 2006-08-08 21:56:50 ghudson Exp $

# If we don't have tickets, or aren't using the default MAILRELAY, we
# can't do authentication.  Otherwise, we have tickets and are using the
# ATHENA.MIT.EDU MX record, and can use authentication.

. /etc/athena/rc.conf

/usr/athena/bin/klist -s

if [ $? != 0 -o "$MAILRELAY" != "default" \
     -o ! -f /etc/athena/sendmail.conf ]; then
  UNAUTH_MAIL=1; export UNAUTH_MAIL
  flags="-U"
else
  flags="-P 587"
fi

exec /usr/sbin/sendmail.real $flags "$@"
