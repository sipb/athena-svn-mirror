#!/bin/sh
# $Id: fix-xconfig.sh,v 1.2 2000-10-04 04:59:08 ghudson Exp $

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
if [ -n "$horiz" ]; then
  runsed "s/\(HorizSync$wsp\)[0-9][0-9,.-]*/\1$horiz/"
fi

vert=`echo "$edid" | awk '/VertRefresh/ { print $2; }'`
if [ -n "$vert" ]; then
  runsed "s/\(VertRefresh$wsp\)[0-9][0-9,.-]*/\1$vert/"
fi

# Second, get rid of any 1600x1200 mode which might be present.
runsed "/^${optwsp}Modes$wsp\"1600x1200\"$optwsp\$/s/1600x1200/1280x1024/"
runsed "/^[ 	]*Modes/s/\"1600x1200\"$optwsp//"
