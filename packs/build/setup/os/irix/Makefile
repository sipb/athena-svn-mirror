# $Id: Makefile,v 1.2 1998-12-26 15:51:28 ghudson Exp $

SHELL=/bin/sh
USRBIN=/usr/bin

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBIN}
	rm -f ${DESTDIR}/core
	ln -s var/athena/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin

clean:

distclean:
