# $Id: Makefile,v 1.1 2004-03-23 20:37:47 rbasch Exp $

SHELL=/bin/sh
ATHMANDIR=/usr/athena/man
ATHRETCDIR=/etc/athena

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHMANDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0444 mandesc ${DESTDIR}${ATHMANDIR}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}

clean:

distclean:
