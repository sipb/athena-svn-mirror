# $Id: Makefile,v 1.2 1998-12-26 15:51:22 ghudson Exp $

SHELL=/bin/sh
CONFIGDIR=/usr/athena/config

all:

check:

install:
	mkdir -p ${DESTDIR}${CONFIGDIR}
	install -c -m 444 Imake.rules ${DESTDIR}${CONFIGDIR}
	install -c -m 444 Imake.tmpl ${DESTDIR}${CONFIGDIR}
	install -c -m 444 mkdirhier.sh ${DESTDIR}${CONFIGDIR}
	install -c -m 444 site.def ${DESTDIR}${CONFIGDIR}
	rm -f ${DESTDIR}${CONFIGDIR}/cf
	ln -s . ${DESTDIR}${CONFIGDIR}/cf

clean:

distclean:
