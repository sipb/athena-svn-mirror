#!/bin/sh
# $Id: net-event.sh,v 1.3 2003-06-10 15:14:00 ghudson Exp $
#
# net-event:	the network event script.  This script is called from a
#		number of various system scripts.  The first argument is
#		the event type.  Different event types may have different
#		additional arguments....
#

pidfile="/var/run/athstatusd.pid"
lockfile="/var/run/athstatusd.lock"
networkfile="/var/run/athstatusd.network"
statusfile="/var/run/athstatusd.info"

#
# See if we should run the Disconnected code..
#
if [ -f /etc/athena/rc.conf ]; then
  . /etc/athena/rc.conf
  [ true = "$DISCONNECTABLE" ] || exit 0
fi

# First, check if statusd is running (or was run at some point).  If
# it is not running then queue the event for later processing.
if [ ! -f $pidfile ]; then
  echo "$0 $@" >> /var/run/athstatusd.events
  exit 0
fi

# Make sure we can find some athena-isms
PATH=/usr/athena/bin:/bin/athena:$PATH
export PATH

# Wait for the lock to go away
while [ -f $lockfile ] ; do
  sleep 1
done

# Maybe load the system-wide network configuration script?
if [ -f /etc/athena/network-scripts/net-event.conf ]; then
  . /etc/athena/network-scripts/net-event.conf
fi

#
# usage: get_ipaddr DEVICE
#
# Get the current IP Address of the given interface and 'echo' it.
# exits with '0' on success, '1' if there is an error determining
# the ip address.
#
get_ipaddr() {
  dev="$1"

  line=`env LANG=C ifconfig "$dev" | grep "inet addr"`
  addr=`echo $line | sed -e 's/^.*inet addr:\([^ ]*\).*$/\1/'`
  if [ "x$addr" != "x$line" ]; then
    echo "$addr"
    return 0
  fi
  return 1
}

#
# usage: old_ipaddr DEVICE
#
# check the athinfo.network for the old IP Address for DEVICE
# echo's the address
#
old_ipaddr() {
  dev="$1"

  line=`grep "$dev=" $networkfile`
  echo "$line" | sed -e s/"$dev"=//
  return 0
}

#
# usage: update_network STATUS DEVICE
#
# updates the network file.  Status is "up" or "down"
# and the Device is the network device to handle.
#
update_network() {
  status="$1"
  dev="$2"

  # make sure the file exists
  touch $networkfile

  # now remove any pre-existing information on this device
  ( echo "/$dev=/" ; echo d ; echo wq ) | ed $networkfile >/dev/null 2>&1

  # if this is an 'up', then add the device (to the end)
  if [ up = "$status" ]; then
    echo "$dev=`ip_addr $dev`" >> $networkfile
  fi
}

#
# usage: set_hostname DEVICE
#
# This verifies a net-up event and makes sure it's
# a "reasonable" network and exit's the script if it is not.
#
set_hostname() {
  dev="$1"

  # skip lo, dummy, and ipsec devices
  case "$dev" in
    lo*|dummy*|ipsec*)
      rm -f $lockfile
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
      rm -f $lockfile
      exit 0
    fi
  fi

  # ok, figure out the hostname and set it if we find it.
  if addr=`get_ipaddr "$dev"`; then
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
touch $lockfile
rm -f $statusfile

# start the status file
touch $statusfile
echo "EVENT=$1" > $statusfile
echo "OLDHOSTNAME=`hostname`" >> $statusfile

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
    echo NETDEV="$dev" >> $statusfile
    echo OLDIPADDR="`old_ipaddr $dev`" >> $statusfile
    echo IPADDR="`ip_addr $dev`" >> $statusfile
    if [ ! -f $networkfile ]; then
      echo "FIRST_NET_UP=true" >> $statusfile
    fi
    update_network up $dev
    echo "NETDEVCOUNT=`wc -l $networkfile`" >> $statusfile
    ;;

  net-down)
    echo NETDEV="$2" >> $statusfile
    update_network down $dev
    echo "NETDEVCOUNT=`wc -l $networkfile`" >> $statusfile
    ;;

  *)
    have_event=""
    ;;
esac

echo "HOSTNAME=`hostname`" >> $statusfile

# Run the system scripts
if [ -d /etc/athena/network-scripts/system ]; then
  for f in /etc/athena/network-scripts/system/*; do
    if [ -x "$f" ]; then
      "$f" $statusfile
    fi
  done
fi

# Signal the status daemon to wake up everyone.
if [ -n "$have_event" -a -f $pidfile ]; then
  kill -USR1 `cat $pidfile` || rm -f $lockfile
else
  # No signaling -- unlock ourselves
  rm -f $lockfile
fi

exit 0
