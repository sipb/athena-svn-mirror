# $Id: Makefile,v 1.6 1997-10-06 16:17:55 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin

all:

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${ATHBINDIR}/perl
	ln -s /usr/sbin/perl ${DESTDIR}${ATHBINDIR}/perl

clean:

distclean:
