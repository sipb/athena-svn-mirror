# $Id: Makefile,v 1.4 1998-01-31 23:37:13 ghudson Exp $

SHELL=/bin/sh

all:

check:

install:
	-mkdir -p ${DESTDIR}/usr/bin
	rm -f ${DESTDIR}/core
	ln -s var/athena/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin

clean:

distclean:
