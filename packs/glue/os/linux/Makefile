# $Id: Makefile,v 1.4 2002-10-22 20:14:15 zacheiss Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ${ATHBINDIR}/lp ${DESTDIR}${USRBINDIR}/lp

clean:

distclean:
