#!/bin/sh
# $Id: fix-xconfig.sh,v 1.1 2000-09-04 16:49:10 ghudson Exp $

# fix-xconfig: Modify the X configuration to match timings retrieved
# from read-edid and to have the desired maximum resolution.  Run from
# athena-ws.rc on PUBLIC=true machines.

PATH=/etc/athena:/bin:/usr/bin
cfg=/etc/X11/XF86Config

if [ ! -f $cfg ]; then
  exit
fi

runsed() {
  sed -e "$1" $cfg > $cfg.new
  if [ -s $cfg.new ]; then
    mv -f $cfg.new $cfg
  fi
}

# First, fix up the horizontal and vertical sync rates.

edid=`get-edid | parse-edid 2>/dev/null`

horiz=`echo "$edid" | awk '/HorizSync/ { print $2; }'`
if [ -n "$horiz" ]; then
  runsed "s/\(HorizSync[ 	][ 	]*\)[0-9][0-9,.-]*/\1$horiz/"
fi

vert=`echo "$edid" | awk '/VertRefresh/ { print $2; }'`
if [ -n "$vert" ]; then
  runsed "s/\(VertRefresh[ 	][ 	]*\)[0-9][0-9,.-]*/\1$vert/"
fi

# Second, get rid of any 1600x1200 mode which might be present.
runsed '/^[ 	]*Modes/s/"1600x1200" *//'
