# $Id: Makefile,v 1.2 1996-11-23 17:42:25 ghudson Exp $

SHELL=/bin/sh
SYSTEM=/usr/athena/lib/init

all:

check:

install:
	-mkdir -p ${DESTDIR}${SYSTEM}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:
