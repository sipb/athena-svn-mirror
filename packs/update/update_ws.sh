#!/bin/sh
# $Id: update_ws.sh,v 1.65 2003-07-15 06:01:10 ghudson Exp $

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
PATH=/bin:/etc:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/etc/athena:/bin/athena:/os/usr/bin:/usr/athena/sbin:/os/usr/ucb:/os/usr/bsd:/usr/sbin:$LIBDIR
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
version=`awk '$0 != "" {a=$5}; END{print a}' "$CONFDIR/version"`

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
    AUTOUPDATE=false getcluster -b -l /etc/athena/cluster.local \
      "$HOST" "$version" > /var/athena/cluster.oldrel
    if [ -s /var/athena/cluster.oldrel ]; then
      . /var/athena/cluster.oldrel
    fi
    attach -O "$SYSLIB"
  fi
  exit 1
}

# This check needs to be updated for each full release.  It verifies
# that people aren't updating across two full releases, which
# generally doesn't work.
case "$version" in
9.1.*|9.2.*)
  ;;
*)
  echo "You must update by only one full release at a time."
  failupdate
  ;;
esac

case "$HOSTTYPE" in
sun4)

  # Set required filesystem sizes 9.2 release, and required filesystem
  # space for the 9.2 update.  Filesystem overhead consumes about 7% of
  # a partition, so we check for a filesystem size 10% less than the
  # partition size we want.  We get some extra margin from minfree,
  # but we don't deliberately rely on that.

  if [ -h /usr/athena ]; then
    # Multi-partition Sun.
    reqrootsize=184320	# 200MB partition; measured use 92698K
    requsrsize=184320	# 200MB partition; measured use 160819K
    reqrootspace=30720	# 30MB; measured space increase 26338K
    requsrspace=10240	# 10MB; measured space increase 3582K
  else
    # Single-partition Sun.
    reqrootsize=2097152	# 2GB partition; measured use 1021038K
    requsrsize=0
    reqrootspace=819200	# 800MB; measured space increase 708325K
    requsrspace=0
  fi

  # Check filesystem sizes.
  rootsize=`df -k / | awk '{ x = $2; } END { print x; }'`
  usrsize=`df -k /usr | awk '{ x = $2; } END { print x; }'`
  if [ "$reqrootsize" -gt "$rootsize" -o "$requsrsize" -gt "$usrsize" ]; then
    echo "Your / or /usr partition is not big enough for Athena release 9.1"
    echo "and higher.  You must reinstall to take this update."
    logger -t "$HOST" -p user.notice / or /usr too small to take update
    failupdate
  fi

  # Check free space if this is a full update to 9.1.
  case $version in
  9.1.*)
    rootspace=`df -k / | awk '{ x = $4; } END { print x; }'`
    usrspace=`df -k /usr | awk '{ x = $4; } END { print x; }'`
    if [ "$reqrootspace" -gt "$rootspace" \
	 -o "$requsrspace" -gt "$usrspace" ]; then
      echo "The / partition must have ${reqrootspace}K free and the /usr"
      echo "partition must have ${requsrspace}K free for this update.  Please"
      echo "reinstall or clean local files off the / and /usr partitions."
      logger -t "$HOST" -p user.notice / or /usr too full to take update
      failupdate
    fi
    ;;
  9.2.?)
    # 9.2.10 introduced 172MB of new data on srvd.big machines.
    if [ -h /usr/athena ]; then
      rootspace=`df -k / | awk '{ x = $4; } END { print x; }'`
      if [ 204800 -gt "$rootspace" ]; then
        echo "The / partition must have ${reqrootspace}K free for this update."
        echo "Please clean local files off the disk."
        logger -t "$HOST" -p user.notice / too full to take update
        failupdate
      fi
    fi
    ;;
  esac

  # Ultras with old enough OBP versions aren't able to boot the 64 bit
  # Solaris kernel without a firmware upgrade.  They will fail the 
  # update ungracefully, since the miniroot can only boot the 64 bit
  # kernel on Ultras.  Check the version of OBP here so we can bomb out
  # gracefully.
  #
  # We must be running version 3.11.1 or greater in order to be able to
  # boot the 64 bit kernel.
  if [ sun4u = `uname -m` ]; then
    eval `prtconf -V | awk '{print $2}' \
      | awk -F. '{print "obpmajor=" $1, "obpminor=" $2}'`
    if [ ! "$obpmajor" -gt 3 -a ! "$obpminor" -ge 11 ]; then
      echo "This machine requires a firmware upgrade for this update."
      logger -t "$HOST" -p user.notice firmware too old to take update
      failupdate
    fi
  fi

  # Athena 9.1 does not support sun4m hardware.
  if [ sun4m = `uname -m` ]; then
    echo "This machine is no longer supported and is too old for this update."
    logger -t "$HOST" -p user.notice sun4m hardware unable to take update
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
    logger -t "$HOST" -p user.notice missing mkserv services, unable to take update
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
