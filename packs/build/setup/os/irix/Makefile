# $Id: Makefile,v 1.5 1999-11-06 21:14:16 rbasch Exp $

SHELL=/bin/sh
USRBIN=/usr/bin
USRSBIN=/usr/sbin

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBIN}
	mkdir -p ${DESTDIR}${USRSBIN}
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin
	install -c -m 755 postinst ${DESTDIR}${USRSBIN}
	mkdir -p ${DESTDIR}/usr/local/lib/netscape/plugins
	rm -f ${DESTDIR}/usr/local/lib/netscape/plugins/libcosmoplayer.so
	ln -s \
	  ../../../../../var/netscape/communicator/plugins/libcosmoplayer.so \
	  ${DESTDIR}/usr/local/lib/netscape/plugins/libcosmoplayer.so

clean:

distclean:
