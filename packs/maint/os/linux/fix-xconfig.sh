#!/bin/sh
# $Id: fix-xconfig.sh,v 1.3 2000-10-14 17:31:34 ghudson Exp $

# fix-xconfig: Modify the X configuration to match timings retrieved
# from read-edid and to have the desired maximum resolution.  Run from
# athena-ws.rc on PUBLIC=true machines.

PATH=/etc/athena:/bin:/usr/bin
cfg=/etc/X11/XF86Config

if [ ! -f $cfg ]; then
  exit
fi

# Some helper stuff for running sed.
runsed() {
  sed -e "$1" $cfg > $cfg.new
  if [ -s $cfg.new ]; then
    mv -f $cfg.new $cfg
  fi
}
optwsp=`printf "[ \t]*"`
wsp=`printf "[ \t][ \t]*"`

# First, fix up the horizontal and vertical sync rates.
edid=`get-edid 2>/dev/null | parse-edid 2>/dev/null`

horiz=`echo "$edid" | awk '/HorizSync/ { print $2; }'`
vert=`echo "$edid" | awk '/VertRefresh/ { print $2; }'`

if echo "$edid" | grep -q 'Identifier "HP 90 Monitor"'; then
  # Special-case; doesn't seem to actually live up to its promises.
  horiz=31.5-64.3
  vert=50-95
fi

if [ -n "$horiz" ]; then
  runsed "s/\(HorizSync$wsp\).*/\1$horiz/"
fi

if [ -n "$vert" ]; then
  runsed "s/\(VertRefresh$wsp\).*/\1$vert/"
fi

# Second, get rid of any 1600x1200 mode which might be present.
runsed "/^${optwsp}Modes$wsp\"1600x1200\"$optwsp\$/s/1600x1200/1280x1024/"
runsed "/^[ 	]*Modes/s/\"1600x1200\"$optwsp//"
