#!/bin/sh
# $Id: xlock.sh,v 1.4.6.1 2000-09-26 14:47:51 ghudson Exp $

if /usr/athena/bin/xss-command -version 2>/dev/null; then
  exec /usr/athena/bin/xss-command -lock
else
  exec /usr/athena/bin/xss -start-locked
fi
