#!/bin/sh
# $Id: syncclock.sh,v 1.1.2.1 2003-10-23 03:47:14 ghudson Exp $

# Sync the hardware clock to the current system time.

# Use local time, unless /etc/sysconfig/clock says to use universal time.
UTC=false
if [ -f /etc/sysconfig/clock ]; then
  . /etc/sysconfig/clock

  # Allow an old-style setting for UTC.
  if [ "$CLOCKMODE" = "GMT" ]; then
    UTC=true
  fi
fi

case "$UTC" in
true|yes)
  timeflag="--utc"
  ;;
*)
  timeflag="--localtime"
  ;;
esac

/sbin/hwclock --systohc "$timeflag"
