# $Id: Makefile,v 1.3.4.1 2002-11-13 19:52:42 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${USRBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ../athena/bin/lp ${DESTDIR}${USRBINDIR}/lp

clean:

distclean:
