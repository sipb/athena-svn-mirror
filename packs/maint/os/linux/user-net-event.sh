#!/bin/sh
# $Id: user-net-event.sh,v 1.1 2002-11-06 20:37:10 ghudson Exp $
#
# user-net-event:	This is run in the user's environment from the
#			neteventd process.  It takes a single argument,
#			which is the path to the status file.  This file
#			should be removed at exit.
#

# Add some athena-isms to the path
PATH=/usr/athena/bin:/bin/athena:$PATH:/usr/X11R6/bin
export PATH

# Load the status file
if [ -f "$1" ]; then
  . "$1"
fi

#
# This case statement is a placeholder for hard-coded
# system event handling.  You could, theoretically,
# use it to notify users of the events (by using these
# echo statements.
#
case "$EVENT" in
suspend|net-down)
  #echo "Network event: $EVENT.  Nothing to do"
  ;;

resume)
  #echo "Resume..."
  ;;

net-up)
  #echo "The network is back up..."
  ;;
esac

# Run the user scripts
if [ -d /etc/athena/network-scripts/user ]; then
  for f in /etc/athena/network-scripts/user/*; do
    if [ -x "$f" ]; then
      "$f" "$1"
    fi
  done
fi

# Remove the status file
rm -f "$1"

exit 0
