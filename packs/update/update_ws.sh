#!/bin/sh
# $Id: update_ws.sh,v 1.47 2000-05-19 18:31:10 ghudson Exp $

# Copyright 1996 by the Massachusetts Institute of Technology.
#
# Permission to use, copy, modify, and distribute this
# software and its documentation for any purpose and without
# fee is hereby granted, provided that the above copyright
# notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting
# documentation, and that the name of M.I.T. not be used in
# advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.
# M.I.T. makes no representations about the suitability of
# this software for any purpose.  It is provided "as is"
# without express or implied warranty.

# update_ws (also known as auto_update)
#
# Check that an update is needed, and make sure the conditions necessary
# for a successful update are met. Then prepare the machine for update,
# and run do-update.

# "tee" doesn't work reliably across OS versions (since it's not local on
# Solaris), so emulate it in the shell.
shelltee() {
  exec 3>$1
  while IFS="" read line; do
    echo "$line"
    echo "$line" 1>&3
  done
}

trap "" 1 15

export CONFDIR LIBDIR PATH HOSTTYPE
CONFDIR=/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
PATH=/bin:/etc:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/etc/athena:/bin/athena:/os/usr/bin:/usr/athena/sbin:/os/usr/ucb:/os/usr/bsd:$LIBDIR
HOSTTYPE=`/bin/athena/machtype`

case $0 in
*auto_update)
  method=Auto
  why=$1
  ;;
*)
  method=Manual
  ;;
esac

# The -a option specifies that the update is automatic (run by the
# boot script or reactivate).  The -r option specifies that the update
# is remote and that we shouldn't give the user a shell after the
# reboot.
while getopts a:r opt; do
  case $opt in
  a)
    method=Auto
    why=$OPTARG
    ;;
  r)
    method=Remote
    ;;
  \?)
    echo "$0 [-r] [-a reactivate|rc]" 1>&2
    exit 1
  ;;
  esac
done
shift `expr $OPTIND - 1`

case `id` in
"uid=0("*)
  ;;
*)
  echo "You are not root.  This update script must be run as root."
  exit 1
  ;;
esac

# If /srvd is not mounted, quit.
if [ ! -d /srvd/bin ]; then
  exit 1
fi

if [ ! -f "$CONFDIR/version" ]; then
  echo "Athena Update (???) Version 0.0A Mon Jan 1 00:00:00 EDT 0000" \
    > "$CONFDIR/version"
fi

newvers=`awk '{a=$5}; END{print a}' /srvd/.rvdinfo`
version=`awk '{a=$5}; END{print a}' "$CONFDIR/version"`

# A temporary backward compatibility hack, necessary as long as there are
# 7.7 and 8.0 machines upgrading to the new release.
case "$version" in
[0-9].[0-9][A-Z])
  version=`echo $version | awk '{ print substr($1, 1, 3) "." \
    index("ABCDEFGHIJKLMNOPQRSTUVWXYZ", substr($1, 4, 1)) - 1; }'`
  ;;
esac

if [ -f "$CONFDIR/rc.conf" ]; then
  . "$CONFDIR/rc.conf"
else
  export PUBLIC AUTOUPDATE HOST
  PUBLIC=true
  AUTOUPDATE=true
  HOST=`hostname`
fi

# Make sure /var/athena exists (it was introduced in 8.1.0) so we have a
# place to put the temporary clusterinfo file and the desync state file
# and the update log.
if [ ! -d /var/athena ]; then
  mkdir -m 755 /var/athena
fi

# Get and read cluster information, to set one or both of
# NEW_TESTING_RELEASE and NEW_PRODUCTION_RELEASE if there are new
# releases available.
/etc/athena/save_cluster_info
if [ -f /var/athena/clusterinfo.bsh ]; then
  . /var/athena/clusterinfo.bsh
fi

# Check if we're already in the middle of an update.
case $version in
[0-9]*)
  # If this field starts with a digit, we're running a proper
  # release. Otherwise...
  ;;
*)
  if [ ! -f /var/tmp/update.check ]; then
    logger -t $HOST -p user.notice at revision $version
    touch /var/tmp/update.check
  fi

  echo "This system is in the middle of an update.  Please contact"
  echo "Athena Cluster Services at x3-1410. Thank you. -Athena Operations"
  exit 1
  ;;
esac

# Find out if the version in /srvd/.rvdinfo is newer than
# /etc/athena/version.  Distinguish between major, minor, and patch
# releases so that we can desynchronize patch releases.
packsnewer=`echo "$newvers $version" | awk '{
  split($1, v1, ".");
  split($2, v2, ".");
  if (v1[1] + 0 > v2[1] + 0)
    print "major";
  else if (v1[1] + 0 == v2[1] + 0 && v1[2] + 0 > v2[2] + 0)
    print "minor";
  else if (v1[1] == v2[1] && v1[2] == v2[2] && v1[3] + 0 > v2[3] + 0)
    print "patch"; }'`

# If the packs aren't any newer, print an appropriate message and exit.
if [ -z "$packsnewer" ]; then
  if [ Auto != "$method" ]; then
    # User ran update_ws; display something appropriate.
    if [ -n "$NEW_PRODUCTION_RELEASE" -o \
         -n "$NEW_TESTING_RELEASE" ]; then
      echo "Your workstation software already matches the version on the"
      echo "system packs.  You must manually attach a newer version of the"
      echo "system packs to update beyond this point."
    else
      echo "It appears you already have this update."
    fi
  else
    # System ran auto_update; point out new releases if available.
    if [ -n "$NEW_PRODUCTION_RELEASE" ]; then
      ver=$NEW_PRODUCTION_RELEASE
      echo "A new Athena release ($ver) is available.  Since it may be"
      echo "incompatible with your workstation software, your workstation is"
      echo "still using the old system packs.  Please contact Athena Cluster"
      echo "Services (x3-1410) to have your workstation updated."
    fi
    if [ -n "$NEW_TESTING_RELEASE" ]; then
      ver=$NEW_TESTING_RELEASE
      echo "A new Athena release ($ver) is now in testing.  You are"
      echo "theoretically interested in this phase of testing, but because"
      echo "there may be bugs which would inconvenience your work, you must"
      echo "update to this release manually.  Please contact Athena Cluster"
      echo "Services (x3-1410) if you have not received instructions on how"
      echo "to do so."
    fi
  fi
  exit 0
fi

# The packs are newer, but if we were run as auto_update, we don't want to do
# an update unless the machine is autoupdate (or public).
if [ Auto = "$method" -a true != "$AUTOUPDATE" -a true != "$PUBLIC" ]; then
  echo "A new version of Athena software is now available.  Please contact"
  echo "Athena Cluster Services (x3-1410) to get more information on how to"
  echo "update your workstation yourself, or to schedule us to do it for you." 
  echo "Thank you.  -Athena Operations"
  if [ ! -f /var/tmp/update.check ]; then
    logger -t "$HOST" -p user.notice at revision $version
    cp /dev/null /var/tmp/update.check
  fi
  exit 1
fi

if [ Auto = "$method" -a patch = "$packsnewer" ]; then
  # There is a patch release available and we want to take the update,
  # but not necessarily right now.  Use desync to stagger the update
  # over a four-hour period.  (Use the version from /srvd for now to
  # make sure the -t option works, since that option was not
  # introduced until 8.1.)  Note that we only do desynchronization
  # here for patch releases.  Desynchronization for major or minor
  # releases is handled in getcluster, since we don't want the
  # workstation to run with a new, possibly incompatible version of
  # the packs.

  /srvd/etc/athena/desync -t /var/athena/update.desync 14400
  if [ $? -ne 0 ]; then
    exit 0
  fi
fi

# Beyond this point, if the update fails, it's probably due to a full
# release which we can't take for some reason.  A machine with new
# system packs attached won't function properly in this state, so we
# should attempt to reattach the old system packs before exiting if
# this was an automatic update attempt.  This function takes care of
# reattaching the old system packs and exiting with an error status.
failupdate() {
  if [ Auto = "$method" ]; then
    echo "Attempting to reattach old system packs"
    detach "$SYSLIB"
    AUTOUPDATE=false getcluster -l /etc/athena/cluster.local \
      "$HOST" "$version"` > /var/athena/cluster.oldrel
    if [ -s /var/athena/cluster.oldrel ]; then
      . /var/athena/cluster.oldrel
    fi
    attach -O "$SYSLIB"
  fi
  exit 1
}

case "$HOSTTYPE" in
sun4)
  # Make sure /usr is a 100MB partition, or it can't run the current release.
  # Since roughly 7% of the partition is lost to filesystem overhead, we
  # check for a filesystem size of 90MB or greater.
  usrsize=`df -k /usr | awk '{ x = $2' }' END { print x; }' `
  if [ 92160 -gt "$usrsize" ]; then
    echo "/usr partition is not big enough for Athena release 8.4 and higher."
    echo "You must reinstall to take this update."
    failupdate
  fi

  # For the update to 8.4, ensure that there is at least 35MB
  # available.  Information relevant to this calculation:
  #	* A freshly installed 8.3 machine uses 43797K on /usr.
  #	* A freshly updated 8.4 machine uses 78118K on /usr.
  #	* Roughly 7% of the partition is lost to overhead.
  #	* The df -k available space output reflects another 10%
  #	  reduction on account of minfree.
  # So on a freshly installed 8.3 machine with a 100MB usr partition,
  # there will be roughly 37MB available, and the update will consume
  # roughly 34MB.  This isn't as bad as it sounds; we can go over
  # the available amount as long as we don't exhaust minfree.
  case $version in
  8.[0123].*|7.*)
    usrspace=`df -k /usr | awk '{ x = $4 }' END { print x; }'`
    if [ 35840 -gt "$usrspace" ]; then
      echo "The /usr partition must have 35MB free for this update.  Please"
      echo "reinstall of clean local files off of the /usr partition."
    fi
    ;;
  esac
  ;;
esac

# Ensure that we have enough disk space on the IRIX root partition
# for an OS upgrade.
# We also require a minimum of 64MB of memory on all SGI's.
case $HOSTTYPE in
sgi)
  case `uname -r` in
  6.2)
    rootneeded=130
    ;;
  6.3)
    rootneeded=70
    ;;
  6.5)
    case "`uname -R | awk '{ print $2; }'`" in
    6.5.3m)
      rootneeded=30
      ;;
    *)
      rootneeded=0
      ;;
    esac
    ;;
  *)
    rootneeded=0
    ;;
  esac

  rootfree=`df -k / | awk '$NF == "/" { print int($5 / 1024); }'`
  if [ "$rootfree" -lt "$rootneeded" ]; then
    echo "Root partition low on space (less than ${rootneeded}MB); not"
    echo "performing update.  Please reinstall or clean local files off root"
    echo "partition."
    failupdate
  fi

  if [ 64 -gt "`hinv -t memory | awk '{ print $4; }'`" ]; then
    echo "Insufficient memory (less than 64MB); not performing update.  Please"
    echo "add more memory or reinstall."
    failupdate
  fi
  ;;
esac

# If this is a private workstation, make sure we can recreate the mkserv
# state of the machine after the update.
if [ -d /var/server ] ; then
  /srvd/usr/athena/bin/mkserv updatetest
  if [ $? -ne 0 ]; then
    echo "mkserv services cannot be found for all services.  Update cannot be"
    echo "performed."
    failupdate
  fi
fi

# Tell dm to shut down everything and sleep forever during the update.
if [ Auto = "$method" -a reactivate = "$why" ]; then
  if [ -f /var/athena/dm.pid ]; then
    kill -FPE `cat /var/athena/dm.pid`
  fi
  # 8.0 and prior machines still have /etc/athena/dm.pid.
  if [ -f /etc/athena/dm.pid ]; then
    kill -FPE `cat /etc/athena/dm.pid`
  fi

  if [ -f /etc/init.d/axdm ]; then
    /etc/init.d/axdm stop
  fi

  sleep 2
  if [ sgi = "$HOSTTYPE" ]; then
   exec 1>/dev/tport 2>&1
  fi
fi

# Everything is all set; do the actual update.
rm -f /var/athena/update.log
if [ Auto = "$method" ]; then
  echo
  echo "THIS WORKSTATION IS ABOUT TO UNDERGO AN AUTOMATIC SOFTWARE UPDATE."
  echo "THIS PROCEDURE MAY TAKE SOME TIME."
  echo
  echo "PLEASE DO NOT DISTURB IT WHILE THIS IS IN PROGRESS."
  echo
  exec sh "$LIBDIR/do-update" "$method" "$version" "$newvers" \
    < /dev/null 2>&1 | shelltee /var/athena/update.log
else
  exec sh "$LIBDIR/do-update" "$method" "$version" "$newvers" \
    2>&1 | shelltee /var/athena/update.log
fi
