#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
# $Id: save_cluster_info.sh,v 1.12 1997-04-01 00:52:59 ghudson Exp $
#
# Use old data from last session if getcluster fails.

PATH=/bin:/usr/ucb:/usr/bsd; export PATH

. /etc/athena/rc.conf
VERSION=`awk '{vers = $5}; END {print vers}' /etc/athena/version`

if [ "${VERSION}" = "Update" ]; then
	echo "This system is in the middle of an update."
	echo "Please contact Athena Operations."
	exit 1
fi

/bin/athena/getcluster -b ${HOST} ${VERSION} > /var/athena/clusterinfo.bsh.new
if [ $? -eq 0 -a -s /var/athena/clusterinfo.bsh.new ]
then
	rm -f /var/athena/clusterinfo.bsh
	mv /var/athena/clusterinfo.bsh.new /var/athena/clusterinfo.bsh
	chmod 644 /var/athena/clusterinfo.bsh
fi
/bin/athena/getcluster ${HOST} ${VERSION} > /var/athena/clusterinfo.new
if [ $? -eq 0 -a -s /var/athena/clusterinfo.new ]
then
	rm -f /var/athena/clusterinfo
	mv /var/athena/clusterinfo.new /var/athena/clusterinfo
	chmod 644 /var/athena/clusterinfo
fi
