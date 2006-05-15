#!/bin/sh
# $Id: fix-xconfig.sh,v 1.5 2006-05-15 20:09:22 amb Exp $

# fix-xconfig: Modify the X configuration to match timings retrieved
# from read-edid and to have the desired maximum resolution.  Run from
# athena-ws.rc on PUBLIC=true machines.

PATH=/etc/athena:/bin:/usr/bin:/sbin
cfg=/etc/X11/xorg.conf

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

if lspci -n|grep -q '102b:0525' ; then
  # Special-case; the Matrix G400 series often can't really do
  # horizontal sync rates over 90, despite claiming otherwise.
  horiz=30-90
fi

if [ -n "$horiz" ]; then
  runsed "s/\(HorizSync$wsp\).*/\1$horiz/"
fi

if [ -n "$vert" ]; then
  runsed "s/\(VertRefresh$wsp\).*/\1$vert/"
fi

# Second, get rid of any 1600x1200 mode which might be present, switching
# it to 1280x1024 if it's the only mode present.
runsed "/^${optwsp}Modes$wsp\"1600x1200\"$optwsp\$/s/1600x1200/1280x1024/"
runsed "/^[ 	]*Modes/s/\"1600x1200\"$optwsp//"

# Add a ServerFlags section with the DontZap flag set, if not already present.
if ! grep -qi "^${optwsp}Section${wsp}\"ServerFlags\"${optwsp}$" $cfg ; then
  cat >> $cfg <<EOF

Section "ServerFlags"
	Option "DontZap"  "on"
EndSection
EOF
fi
