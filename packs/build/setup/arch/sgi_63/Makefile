# $Id: Makefile,v 1.2 1998-07-07 21:17:24 rbasch Exp $

SHELL=/bin/sh
USRBSD=/usr/bsd
USRSBIN=/usr/sbin

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBSD}
	mkdir -p ${DESTDIR}${USRSBIN}
	install -c -m 755 compress ${DESTDIR}${USRBSD}
	install -c -m 755 postinst ${DESTDIR}${USRSBIN}

clean:

distclean:
