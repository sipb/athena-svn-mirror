#!/bin/sh

# $Id: mksessiondir.sh,v 1.1 2003-02-19 23:53:24 rbasch Exp $
# This script creates a temporary directory for the user session,
# ensuring that the directory does not already exist.  It outputs
# the name of the created directory.

progname="`basename $0`"
sid=$$

usage() {
  echo "Usage: $progname [-s <session_ID>]" 1>&2
  exit 1
}

while getopts s: opt; do
  case "$opt" in
  s)
    sid=$OPTARG
    ;;
  \?)
    usage
    ;;
  esac
done

# First try to create the default directory name, based only on the
# user name and session ID.  If that fails, we will use this name as
# the prefix in finding a unique directory name.
dir="/tmp/session-${USER}-${sid}"
mkdir -m 700 "$dir" > /dev/null 2>&1
if [ $? -ne 0 ]; then
  # Default name failed, try to uniquify.
  prefix="$dir"
  maxtries=500
  i=0
  while [ `expr $i \< $maxtries` -eq 1 ]; do
    dir="${prefix}-`expr $$ + $i`"
    mkdir -m 700 "$dir" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
      break
    fi
    i=`expr $i + 1`
  done
  if [ `expr $i \>= $maxtries` -eq 1 ]; then
    echo "$progname: Cannot create a directory" 1>&2
    exit 1
  fi
fi
echo "$dir"
exit 0
