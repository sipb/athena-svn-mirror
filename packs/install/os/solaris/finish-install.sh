#!/bin/sh
# $Id: finish-install.sh,v 1.1 2003-09-12 01:52:03 ghudson Exp $

echo "Starting the second stage of the install at `date`."

# We get one argument, the workstation version we're installing at.
vers="$1"

echo "Creating config files for Athena software"
sh /srvd/install/athchanges

echo "Updating version"
hosttype=`/bin/athena/machtype`
echo "Athena Workstation ($hosttype) Version $vers `date`" \
  >> /etc/athena/version

echo "Cleaning up initial system packs symlinks"
rm -f /srvd /os

echo "Finished with install at `date`."
