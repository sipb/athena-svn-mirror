#!/bin/sh
# $Id: gathrun.sh,v 1.1 2001-05-06 15:53:38 ghudson Exp $

# gathrun - Run a program, or put up an error dialog saying that it
# doesn't exist.

case $# in
0)
  echo "Usage: gathrun locker [program] [args ...]" >&2
  exit 1
  ;;
1)
  locker=$1
  program=$1
  ;;
*)
  locker=$1
  program=$2
  shift 2
  ;;
esac

if attachandrun --check "$locker" "$program" "$@"; then
  exec /bin/athena/attachandrun "$locker" "$program" "$program" "$@"
else
  gdialog --msgbox \
    "$program is not available in the $locker locker on this platform." 0 100
fi
