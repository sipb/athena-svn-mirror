#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
# $Id: save_cluster_info.sh,v 1.2 2003-04-12 16:17:32 ghudson Exp $
#
# Use old data from last session if getcluster fails.

PATH=/bin/athena:/bin:/usr/bin:/usr/ucb:/usr/bsd; export PATH

vers=`awk '{vers = $5}; END {print vers}' /etc/athena/version`

if [ update = "$vers" ]; then
  echo "This system is in the middle of an update." >&2
  echo "Please contact Athena Operations." >&2
  exit 1
fi

# Source existing clusterinfo file to get update timestamp, if any.
if [ -s /var/athena/clusterinfo.bsh ]; then
  . /var/athena/clusterinfo.bsh
fi

getcluster -b "$vers" > /var/athena/clusterinfo.bsh.new
if [ $? -eq 0 -a -s /var/athena/clusterinfo.bsh.new ]; then
  rm -f /var/athena/clusterinfo.bsh
  mv /var/athena/clusterinfo.bsh.new /var/athena/clusterinfo.bsh
  chmod 644 /var/athena/clusterinfo.bsh
fi

getcluster "$vers" > /var/athena/clusterinfo.new
if [ $? -eq 0 -a -s /var/athena/clusterinfo.new ]; then
  rm -f /var/athena/clusterinfo
  mv /var/athena/clusterinfo.new /var/athena/clusterinfo
  chmod 644 /var/athena/clusterinfo
fi
