# $Id: Makefile,v 1.1 1998-12-25 19:33:35 ghudson Exp $

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
