#!/bin/sh
# $Id: local-netscape.sh,v 1.6 2002-09-10 16:08:50 ghudson Exp $

# local-netscape: A cron job to copy part of the infoagents locker onto
# local disk so that netscape can be started more quickly.

PATH=/usr/xpg4/bin:/bin:/usr/bin:/sbin:/usr/sbin:/bin/athena:/usr/athena/bin:/usr/athena/etc
export PATH

local=/var/athena/infoagents
rconf=/var/athena/rconf.infoagents

# Attach the infoagents locker and get its mountpoint.
locker=`attach -pn infoagents`
if [ -z "$locker" ]; then
  exit
fi

# Make sure we have 100MB of space available under /var if this is the
# first time we're bringing netscape local.  Note that df's -P option
# is only supported by /usr/xpg4/bin/df on Solaris 7; thus the
# /usr/xpg4/bin at the beginning of the PATH above.
space=`df -kP /var/athena | awk '{ x = $4; } END { print x; }'`
if [ ! -d "$local" -a "$space" -lt 102400 ]; then
  exit
fi

# Remove netscape.adjusted until everything is properly set up.
# /usr/athena/bin/netscape will use the infoagents locker in the
# meantime.
bin=`athdir $local`
if [ -n "$bin" ]; then
  rm -f "$bin/netscape.adjusted"
  rm -f "$bin/mozilla.adjusted"
fi

version=1
if [ -r "$locker/.syncversion" ]; then
  if [ `cat "$locker/.syncversion"` -gt $version ]; then
    exit
  fi
fi

# Write out the rconf file.
arch=`athdir -t "" -p "$locker" | sed -e "s,^$locker/,," -e 's,/$,,'`
if [ -z "$arch" ]; then
  exit
fi
rm -f $rconf
cat > $rconf << EOF
delete *
copy www -f
copy www/netscape -f
copy www/netscape/* -f
copy share -f
copy share/Netscape -f
copy share/Netscape/* -f
copy share/app-defaults -f
copy share/app-defaults/* -f
copy arch -f
copy arch/share -f
copy arch/share/* -f
chase $arch
copy $arch -f
copy $arch/* -f
delete $arch/MIT-only/*
chase $arch/MIT-only/netscape
copy $arch/MIT-only/netscape -f
copy $arch/MIT-only/netscape/* -f
chase $arch/MIT-only/mozilla
copy $arch/MIT-only/mozilla -f
copy $arch/MIT-only/mozilla/* -f
EOF

# Do the synctree of the locker.
mkdir -p $local
synctree -q -nosrcrules -nodstrules -s "$locker" -d $local -a $rconf
if [ $? -ne 0 ]; then
  rm -rf "$local"
  exit
fi

# If we don't have a netscape binary, assume our synctree failed partially
# and remove our local copy.
if [ ! -f "$local/$arch/MIT-only/netscape/netscape" ]; then
  rm -rf "$local"
  exit
fi

# Make sure the local copy of the locker is readable.
find $local \( -type f -o -type d \) -print | xargs chmod a+rX

# Massage the netscape startup script.
script=$local/arch/share/bin/netscape
if [ -x "$script" ]; then
  rm -f "$script.adjusted"
  sed -e "s,$locker,$local,g" -e 's,^progname=.*$,progname=netscape,' \
    "$script" > $script.new
  chmod a+x "$script.new"
  mv "$script.new" "$script.adjusted"
fi

# Massage the mozilla startup script.
script=$local/arch/share/bin/mozilla
if [ -x "$script" ]; then
  rm -f "$script.adjusted"
  sed -e "s,$locker,$local,g" -e 's,^progname=.*$,progname=mozilla,' \
    "$script" > $script.new
  chmod a+x "$script.new"
  mv "$script.new" "$script.adjusted"
fi
