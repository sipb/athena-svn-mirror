#!/bin/sh
# $Id: gathrun.sh,v 1.3 2004-03-06 17:18:57 ghudson Exp $

# gathrun - Run a program, or put up an error dialog saying that it
# doesn't exist.

splash=no
if [ "$1" = "--splash" ]; then
  shift
  splash=yes
fi

# Set sane values
export ATHENA_SYS=${ATHENA_SYS:-$(machtype -S)}
export ATHENA_SYS_COMPAT=${ATHENA_SYS_COMPAT:-$(machtype -C)}

case $# in
0)
  echo "Usage: gathrun locker [program [args ...]]" >&2
  exit 1
  ;;
1)
  locker=$1
  program=$1
  shift
  ;;
*)
  locker=$1
  program=$2
  shift 2
  ;;
esac

if attachandrun --check "$locker" "$program" "$@"; then
  if [ "$splash" = "yes" ]; then
    yes | zenity --progress --text="Launching $program, please wait.\n(This may take a few moments)" --pulsate --no-cancel --timeout 7 &
  fi
  exec /bin/athena/attachandrun "$locker" "$program" "$program" "$@"
else
  zenity --info \
    --text="$program is not available in the $locker locker on this platform."
fi
