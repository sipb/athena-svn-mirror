# $Id: Makefile,v 1.1 1998-02-23 23:36:56 rbasch Exp $

SHELL=/bin/sh
SYSTEM=/usr/athena/lib/init

all:

check:

install:
	-mkdir -p ${DESTDIR}${SYSTEM}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0755 Xsession.dt ${DESTDIR}${SYSTEM}

clean:

distclean:
