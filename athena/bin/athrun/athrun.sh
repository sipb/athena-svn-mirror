#!/bin/sh
# $Id: athrun.sh,v 1.1 2000-03-25 19:05:26 ghudson Exp $

# A user front end to attachandrun.
# "athrun moira" runs moira from the moira locker.
# "athrun gnu gls -l" runs gls -l from the gnu locker.

case $# in
0)
  echo "Usage: athrun locker [program] [args ...]" >&2
  exit 1
  ;;
1)
  exec /bin/athena/attachandrun "$1" "$1" "$1"
  ;;
*)
  locker=$1
  program=$2
  shift 2;
  exec /bin/athena/attachandrun "$locker" "$program" "$program" "$@"
  ;;
esac
