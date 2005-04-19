#!/bin/sh

# $Id: build-registries.sh,v 1.1 2005-04-19 15:44:58 rbasch Exp $

# Build the xpcom and chrome registries for Firefox.

MOZILLA_FIVE_HOME=/usr/athena/lib/firefox
LD_LIBRARY_PATH=$MOZILLA_FIVE_HOME${LD_LIBRARY_PATH:+":$LD_LIBRARY_PATH"}
export MOZILLA_FIVE_HOME LD_LIBRARY_PATH

umask 022

# Run a command, and wait up to a minute for it to complete.
timeout=60
run_with_timeout() {
  cmd="$@"
  $cmd &
  child=$!
  count=0
  while kill -0 $child 2>/dev/null ; do
    count=`expr $count + 1`
    if [ $count -gt $timeout ]; then
      echo "$cmd timed out" 1>&2
      kill -9 $child 2>/dev/null
      break
    fi
    sleep 1
  done
}

if [ -x "$MOZILLA_FIVE_HOME/regxpcom" ]; then
  # Remove any previously generated files.
  rm -rf $MOZILLA_FIVE_HOME/chrome/overlayinfo
  rm -f $MOZILLA_FIVE_HOME/chrome/*.rdf
  rm -f $MOZILLA_FIVE_HOME/components/compreg.dat
  rm -f $MOZILLA_FIVE_HOME/components/xpti.dat

  mkdir "$MOZILLA_FIVE_HOME/chrome/overlayinfo" || exit 1

  # Run regxpcom.
  run_with_timeout "$MOZILLA_FIVE_HOME/regxpcom"

  # Run regchrome.
  run_with_timeout "$MOZILLA_FIVE_HOME/regchrome"
fi
