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

if [ ! -s /var/athena/counter-id ]; then
  case `uname` in
  Linux)
    # The output of "hostid" is derived from the machine's IP address,
    # which isn't the end of the world, but it could result in
    # duplicate counter IDs when two machines generate their IDs on
    # the same address (due to DHCP, cluster-services install
    # addresses, etc.).  Use the MAC address of the first network
    # device listed in NETDEV, if possible; then fall back to hostid.
    netdev=`. /etc/athena/rc.conf; echo "${NETDEV%%,*}"`
    id=`ifconfig "$netdev" | sed -ne 's/://g' -e 's/^.*HWaddr \(.*\)$/\1/p'`
    : ${id:=`hostid`}
    ;;
  SunOS)
    # The output of "hostid" is taken from the CPU board's ID prom, so
    # should be unique across machines.
    id=`hostid`
    ;;
  esac
  echo "$id" > /var/athena/counter-id
  chmod 644 /var/athena/counter-id
fi

host=`hostname`
type=`/bin/athena/machtype`
ctype=`/bin/athena/machtype -c`
version=`awk '/./ { a = $5; } END { print a; }' /etc/athena/version`
id=`cat /var/athena/counter-id`
annot=$1

logger -p user.notice "counterlog: $host $type $ctype $version $id $annot"
