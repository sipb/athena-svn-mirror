# $Id: Makefile,v 1.2 1996-12-27 21:48:10 ghudson Exp $

INITD=/etc/init.d
RC0=/etc/rc0.d
RC2=/etc/rc2.d

all:

check:

install:
	mkdir -p ${DESTDIR}${INITD}
	mkdir -p ${DESTDIR}${RC0}
	mkdir -p ${DESTDIR}${RC2}
	install -c -m 0755 athena ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 nfs.client ${DESTDIR}${INITD}
	install -c -m 0755 rpc ${DESTDIR}${INITD}
	rm -f ${DESTDIR}${RC0}/K90athena
	ln -s ../init.d/athena ${DESTDIR}${RC0}/K90athena
	rm -f ${DESTDIR}${RC2}/S90athena
	ln -s ../init.d/athena ${DESTDIR}${RC2}/S90athena
	rm -f ${DESTDIR}${RC2}/S70finish-update
	ln -s ../init.d/finish-update ${DESTDIR}${RC2}/S70finish-update

clean:

distclean:
