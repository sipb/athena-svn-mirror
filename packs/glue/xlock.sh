#!/bin/sh
# $Id: xlock.sh,v 1.6 2004-05-16 21:29:05 ghudson Exp $

if /usr/athena/bin/xscreensaver-command -version 2>/dev/null; then
  exec /usr/athena/bin/xscreensaver-command -lock
else
  exec /usr/athena/bin/xscreensaver -start-locked
fi
