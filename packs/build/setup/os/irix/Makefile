# $Id: Makefile,v 1.3.2.1 1999-11-08 23:03:25 ghudson Exp $

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
