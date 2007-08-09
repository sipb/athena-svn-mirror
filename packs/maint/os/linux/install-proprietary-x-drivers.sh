#!/bin/sh

# install-proprietary-x-drivers: Install and/or build proprietary ATI
# graphics drivers and the necessary configuration changes to use them.

# Run this at boot time in a non-PUBLIC=true-constrained way so that it
# can maintain drivers for non-public machines that have opted in for
# this scheme.

. /etc/athena/rc.conf
. /var/athena/clusterinfo.bsh 
athenaversion=`awk '{ a = $5; } END { print a; }' /etc/athena/version`

PATH=/etc/athena:/bin:/usr/bin:/sbin
cfg=/etc/X11/xorg.conf
kver=`uname -r`
forceati=false

if [ "$1" = -forceati ] ; then
  forceati=true
  echo "Force-installing ATI driver."
fi

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

# Switch X config to use ATI's fglrx driver:
if [ "$PUBLIC" = true ] && lspci -n|egrep -q '(1002:7183|1002:5b62)' \
    && ! grep -q fglrx ${cfg} || [ "${forceati}" = true ] ; then
  if ! test -f ${cfg}.non-ati ; then
    cp ${cfg} ${cfg}.non-ati
  fi
  runsed "s/\(Driver$wsp\)\"vesa\".*/\1\"fglrx\" # Athena-maintained/" && \
  runsed "s/\(Board[Nn]ame$wsp\).*/\1\"ATI proprietary driver\" # Athena-maintained/"
fi

# Check for presence of fglrx driver in xorg.conf; if present, install
# appropriate kernel module, libraries, and support files, and bash the
# appropriate libGL symlinks.
if grep -q 'fglrx.*Athena-maint' $cfg ; then
    install -m 0744 $SYSPREFIX/config/$athenaversion/fglrx/fglrx-$kver.ko \
	/lib/modules/${kver}/kernel/drivers/char/drm/fglrx.ko \
	&& depmod
    install -m 0644 $SYSPREFIX/config/$athenaversion/fglrx/lib/*.a  /usr/X11R6/lib
    install -m 0755 $SYSPREFIX/config/$athenaversion/fglrx/lib/*.so.*  /usr/X11R6/lib
    install -m 0444 $SYSPREFIX/config/$athenaversion/fglrx/fglrx_dri.so \
	/usr/X11R6/lib/modules/dri/fglrx_dri.so
    install -m 0444 $SYSPREFIX/config/$athenaversion/fglrx/fglrx_drv.o \
	/usr/X11R6/lib/modules/drivers/fglrx_drv.o
    install -m 0444 $SYSPREFIX/config/$athenaversion/fglrx/libfglrxdrm.a \
	/usr/X11R6/lib/modules/linux/libfglrxdrm.a
    install -m 0755 -d /etc/ati
    install -m 0644 $SYSPREFIX/config/$athenaversion/fglrx/ati/* /etc/ati
    cd /usr/X11R6/lib && ln -fs libGL.fgl.so.1.2 libGL.so.1
    cd /usr/X11R6/lib && ln -fs libGL.fgl.so.1.2 libGL.so
fi
