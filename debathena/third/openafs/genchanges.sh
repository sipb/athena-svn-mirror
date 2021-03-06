#!/bin/sh
# genchanges.sh - generate a changes file for a deb file generated via
#	the make-kpkg utility

# KSRC, KVERS, KMAINT, and KEMAIL are expected to be passed through the
# environment.

set -e
umask 022

MODVERS=`cat debian/VERSION | sed s/:/\+/`
ARCH=`dpkg --print-architecture`

mprefix=`grep Package: debian/control.in | cut -d' ' -f 2 | cut -d= -f 1`
chfile="$KSRC/../$mprefix${KVERS}${INT_SUBARCH}_${MODVERS}_${ARCH}.changes"

dpkg-genchanges -b ${KMAINT:+-m"$KMAINT <$KEMAIL>"} -u"$KSRC/.." \
    -cdebian/control > "$chfile"
#debsign "$chfile"

