#!/bin/sh

# Usage: counterlog [annotation]

# Syslog a message at user.notice (which goes to wslogger.mit.edu in
# the normal Athena configuration) containing the hostname, machtype,
# cputype, Athena version, system identifier, and the annotation if
# specified.

# The system identifier is a random number chosen the first time this
# script runs.  It is intended to help distinguish machines which
# change IP addresses due to DHCP.

# This script is run from the Athena boot script and from a root cron
# job.

# If you don't want this logging to happen, touch
# /etc/athena/no-counterlog.

if [ -f /etc/athena/no-counterlog ]; then
  exit 0
fi

if [ ! -f /var/athena/counter-id ]; then
  if [ -r /dev/urandom ]; then
    dd if=/dev/urandom bs=100 count=1 2>/dev/null | sum | awk '{print $1}' \
      > /var/athena/counter-id
  else
    echo "" | awk '{srand; printf "%05d\n", int(rand * 99999); }' \
      > /var/athena/counter-id
  fi
  chmod 644 /var/athena/counter-id
fi

host=`hostname`
type=`/bin/athena/machtype`
ctype=`/bin/athena/machtype -c`
version=`awk '/./ { a = $5; } END { print a; }' /etc/athena/version`
id=`cat /var/athena/counter-id`
annot=$1

logger -p user.notice "counterlog: $host $type $ctype $version $id $annot"
