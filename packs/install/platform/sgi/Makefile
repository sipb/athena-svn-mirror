# $Id: Makefile,v 1.3 1998-04-21 18:06:26 rbasch Exp $

SHELL=/bin/sh
INSTALL=/install

all:

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	install -c -m 0755 buildkernel ${DESTDIR}${INSTALL}
	install -c -m 0755 netconf ${DESTDIR}${INSTALL}
	install -c -m 0755 oschanges ${DESTDIR}${INSTALL}

clean:

distclean:
