# $Id: Makefile,v 1.3 1998-12-31 03:15:43 rbasch Exp $

SHELL=/bin/sh
USRBIN=/usr/bin
USRSBIN=/usr/sbin

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBIN}
	mkdir -p ${DESTDIR}${USRSBIN}
	rm -f ${DESTDIR}/core
	ln -s var/athena/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin
	install -c -m 755 postinst ${DESTDIR}${USRSBIN}

clean:

distclean:
