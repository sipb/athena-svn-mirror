#!/bin/sh
# $Id: net-event.sh,v 1.2 2003-01-14 17:10:11 ghudson Exp $
#
# net-event:	the network event script.  This script is called from a
#		number of various system scripts.  The first argument is
#		the event type.  Different event types may have different
#		additional arguments....
#

#
# See if we should run the Disconnected code..
#
if [ -f /etc/athena/rc.conf ]; then
  . /etc/athena/rc.conf
  [ true = "$DISCONNECTABLE" ] || exit 0
fi

# First, check if statusd is running (or was run at some point).  If
# it is not running then queue the event for later processing.
if [ ! -f /var/run/athstatusd.pid ]; then
  echo "$0 $@" >> /var/run/athstatusd.events
  exit 0
fi

# Make sure we can find some athena-isms
PATH=/usr/athena/bin:/bin/athena:$PATH
export PATH

# Wait for the lock to go away
while [ -f /var/run/athstatusd.lock ] ; do
  sleep 1
done

# Maybe load the system-wide network configuration script?
if [ -f /etc/athena/network-scripts/net-event.conf ]; then
  . /etc/athena/network-scripts/net-event.conf
fi

set_hostname() {
  dev="$1"

  # skip lo, dummy, and ipsec devices
  case "$dev" in
    lo*|dummy*|ipsec*)
      rm -f /var/run/athstatusd.lock
      exit 0
      ;;
  esac

  # If skip_hostname is set, don't set the hostname at all
  if [ "${skip_hostname+set}" = set ]; then
    return
  fi

  # if set_host_devices is set, see if this device matches
  # the list of devices and only set the hostname for those
  # devices
  if [ "${set_host_devices+set}" = set ]; then
    match=false
    for test_dev in $set_host_devices; do
      if [ "$dev" = "$test_dev" ]; then
	match=true
      fi
    done
    if [ $match = false ]; then
      rm -f /var/run/athstatusd.lock
      exit 0
    fi
  fi

  # ok, figure out the hostname and set it if we find it.
  line=`env LANG=C ifconfig "$dev" | grep "inet addr"`
  addr=`echo $line | sed -e 's/^.*inet addr:\([^ ]*\).*$/\1/'`
  if [ "$addr" != "$line" ]; then
    line=`host $addr`
    if [ `echo "$line" | grep -c "domain name pointer"` = 1 ]; then
      host=`echo "$line" | sed 's#^.*domain name pointer \(.*\)$#\1#'`
      if [ ! -z "$host" -a "$host" != "$line" ]; then
	  host=`echo $host | sed 's;\.*$;;' | tr '[A-Z]' '[a-z]'`
	  hostname $host
      fi
    fi
  fi		
}

# create the lock; remove the status file
touch /var/run/athstatusd.lock
rm -f /var/run/athstatusd.info

# start the status file
touch /var/run/athstatusd.info
echo "EVENT=$1" > /var/run/athstatusd.info
echo "OLDHOSTNAME=`hostname`" >> /var/run/athstatusd.info

# fill in the status file based upon the event type
have_event=yes
case "$1" in
  suspend)
    ;;

  resume)
    ;;

  net-up)
    dev="$2"
    set_hostname "$dev"
    if [ ! -f /var/run/athstatusd.network ]; then
      echo "FIRST_NET_UP=true" >> /var/run/athstatusd.info
    fi
    ;;

  net-down)
    ;;

  *)
    have_event=""
    ;;
esac

echo "HOSTNAME=`hostname`" >> /var/run/athstatusd.info

# Run the system scripts
if [ -d /etc/athena/network-scripts/system ]; then
  for f in /etc/athena/network-scripts/system/*; do
    if [ -x "$f" ]; then
      "$f" /var/run/athstatusd.info
    fi
  done
fi

# Signal the status daemon to wakeup everyone.
if [ "x$have_event" != x -a -f /var/run/athstatusd.pid ]; then
  kill -USR1 `cat /var/run/athstatusd.pid`
  [ $? -eq 0 ] || rm -f /var/run/athstatusd.lock
else
  # No signaling -- unlock ourselves
  rm -f /var/run/athstatusd.lock
fi

[ "$1" = "net-up" ] && touch /var/run/athstatusd.network

exit 0
