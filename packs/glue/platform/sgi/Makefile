# $Id: Makefile,v 1.4 1997-02-19 18:17:14 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
NCS=/var/ncs
FLEXLM=/var/flexlm
SGILICENSE=/afs/athena.mit.edu/system/config/license/sgi

all:

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${NCS}
	rm -f ${DESTDIR}${ATHBINDIR}/perl
	ln -s /usr/sbin/perl ${DESTDIR}${ATHBINDIR}/perl
	rm -f ${DESTDIR}${NCS}/glb_site.txt
	ln -s ${SGILICENSE}/glb_site.txt ${DESTDIR}${NCS}/glb_site.txt
	rm -f ${DESTDIR}${FLEXLM}/license.dat
	ln -s ${SGILICENSE}/license.dat ${DESTDIR}${FLEXLM}/license.dat

clean:

distclean:
