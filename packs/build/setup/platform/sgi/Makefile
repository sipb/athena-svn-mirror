# $Id: Makefile,v 1.3 1996-10-02 07:10:54 ghudson Exp $

SHELL=/bin/sh

all:

check:

install:
	-mkdir -p ${DESTDIR}/usr/bin
	rm -f ${DESTDIR}/core
	ln -s var/tmp/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin

clean:

distclean:
