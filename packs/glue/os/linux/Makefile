# $Id: Makefile,v 1.3.4.2 2002-12-09 22:14:04 ghudson Exp $

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
	install -c -m 555 whatis.sh ${DESTDIR}${ATHBINDIR}/whatis
	rm -f ${DESTDIR}${ATHBINDIR}/apropos
	ln -s whatis ${DESTDIR}${ATHBINDIR}/apropos

clean:

distclean:
