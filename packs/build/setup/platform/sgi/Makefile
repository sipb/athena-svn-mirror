# $Id: Makefile,v 1.5 1998-06-08 14:18:08 ghudson Exp $

SHELL=/bin/sh
USRBIN=/usr/bin
USRETC=/usr/etc

all:

check:

install:
	-mkdir -p ${DESTDIR}${USRETC}
	-mkdir -p ${DESTDIR}${USRBIN}
	install -c -m 555 snmpd ${DESTDIR}${USRETC}
	rm -f ${DESTDIR}/core
	ln -s var/athena/core.root ${DESTDIR}/core
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin

clean:

distclean:
