#!/bin/sh
# $Id: user-xauth.sh,v 1.1 2002-12-09 22:56:13 ghudson Exp $
#
# Fix the xauth keys (if you are using xauth) to match your new
# hostname (if the hostname changed).

. "$1" || exit 1

case "$EVENT" in
net-up)
  # If "hostname" changed, then reset xauth keys.
  if [ "x$OLDHOSTNAME" != "x$HOSTNAME" ]; then
    if [ \( -n "$XAUTHORITY" -a -f "$XAUTHORITY" \) -o \
	 -f "$HOME/.Xauthority" ]; then
      echo "Fixing xauth keys (new hostname: $HOSTNAME)"
      ( xauth -i list | grep "$OLDHOSTNAME" | \
        sed -e "s#^$OLDHOSTNAME#add $HOSTNAME#" | xauth ) > /dev/null
    fi
  fi
  ;;
esac

exit 0
