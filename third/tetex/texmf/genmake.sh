#!/bin/sh
# $Id: genmake.sh,v 1.1 2000-03-17 21:54:42 ghudson Exp $

# genmake.sh - Shell script to generate Makefile.instfiles in this
# directory.

# Direct output to Makefile.instfiles
exec >Makefile.instfiles

echo '# $Id: genmake.sh,v 1.1 2000-03-17 21:54:42 ghudson Exp $'
echo ""
echo "TEXMFDIR=/usr/athena/share/texmf"
echo ""
echo "install:"

find . -type d ! -name CVS ! -name . -print |
	sed -e 's,^./\(.*\)$,	mkdir -p ${DESTDIR}${TEXMFDIR}/\1,'

find . -type d -name CVS -prune -o \
	-type f ! -name genmake.sh ! -name "Makefile.*" -print |
	sed -e 's,^./\(.*\)$,	cp \1 ${DESTDIR}${TEXMFDIR}/\1,'
