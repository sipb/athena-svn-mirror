# $Id: Makefile,v 1.2 1999-03-02 15:45:22 rbasch Exp $

SHELL=/bin/sh
INSTALL=/install

all:

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	install -c -m 0755 ais-options ${DESTDIR}${INSTALL}
	install -c -m 0755 buildkernel ${DESTDIR}${INSTALL}
	install -c -m 0755 docinst ${DESTDIR}${INSTALL}
	install -c -m 0755 netconf ${DESTDIR}${INSTALL}
	install -c -m 0755 oschanges ${DESTDIR}${INSTALL}
	install -c -m 0755 tweakdev ${DESTDIR}${INSTALL}

clean:

distclean:
