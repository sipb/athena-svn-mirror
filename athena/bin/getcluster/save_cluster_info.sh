#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v $
#	$Author: ens $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v 1.1 1987-07-22 16:33:50 ens Exp $
#
# Errors from getcluster guarantee that stdout will be size 0,
# therefore a size # 0 is a sufficient test.
# Use old data from last session if getcluster fails.

/bin/athena/getcluster -b > /tmp/clusterinfo.bsh
if [ -s /tmp/clusterinfo.bsh ]
then
	mv /tmp/clusterinfo.bsh /etc/clusterinfo.bsh
fi
/bin/athena/getcluster > /tmp/clusterinfo
if [ -s /tmp/clusterinfo]
then
	mv /tmp/clusterinfo /etc/clusterinfo
fi
