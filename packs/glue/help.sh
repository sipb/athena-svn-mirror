#!/bin/sh
# $Id: help.sh,v 1.2 2003-10-01 14:03:15 rbasch Exp $

url=file://localhost/afs/athena.mit.edu/astaff/project/olh/index.html

# If we're running on a tty, display a message, since it can take a
# while to start a web browser.  Make sure this doesn't cause a
# suspension if we're backgrounded.  (This might be a little
# unfriendly to people who genuinely want the tostop flag set.  But
# how many of those people run "help"?)

if [ -t 1 ]; then
  stty -tostop
  echo "Starting web browser..."
fi

exec htmlview "$url"
