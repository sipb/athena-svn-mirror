#!/sbin/sh

# $Id: svc-named.sh,v 1.1 2005-04-11 14:11:13 rbasch Exp $

# SMF start method for the Athena BIND daemon.

# Get the SMF exit status definitions.
. /lib/svc/share/smf_include.sh

if [ -x /usr/athena/etc/rndc-confgen ]; then
  # Generate /etc/rndc.key so rndc works.
  /usr/athena/etc/rndc-confgen -a
fi

if [ ! -x /etc/athena/named ]; then
  echo "Cannot execute /etc/athena/named"
  exit $SMF_EXIT_ERR_FATAL
fi

if [ ! -f /etc/named.conf ]; then
  echo "/etc/named.conf is missing"
  exit $SMF_EXIT_ERR_CONFIG
fi

echo "Starting Internet domain name server."
/etc/athena/named

exit $SMF_EXIT_OK
