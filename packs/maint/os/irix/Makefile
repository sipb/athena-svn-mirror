# $Id: Makefile,v 1.5 1999-11-10 03:52:02 kcr Exp $

SHELL=/bin/sh
INITD=/etc/init.d
RC0=/etc/rc0.d
RC2=/etc/rc2.d
ATHRETCDIR=/etc/athena
ATHETCDIR=/usr/athena/etc
CFLAGS=-g

all: os-checkfiles

os-checkfiles: os-checkfiles.o array.o
	${CC} ${LDFLAGS} -o $@ os-checkfiles.o array.o

check:

install:
	mkdir -p ${DESTDIR}${INITD}
	mkdir -p ${DESTDIR}${RC0}
	mkdir -p ${DESTDIR}${RC2}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHETCDIR}
	install -c -m 0755 athena ${DESTDIR}${INITD}
	install -c -m 0755 athena.config ${DESTDIR}${INITD}
	install -c -m 0755 axdm ${DESTDIR}${INITD}
	install -c -m 0755 autoconfig.reboot ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 network ${DESTDIR}${INITD}
	install -c -m 0755 os-checkfiles ${DESTDIR}${ATHETCDIR}
	install -c -m 0755 oscheck.sh ${DESTDIR}${ATHETCDIR}/oscheck
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	install -c -m 0755 xwrapper.sh ${DESTDIR}${ATHRETCDIR}/xwrapper
	rm -f ${DESTDIR}${RC0}/K02xdm
	ln -s ../init.d/axdm ${DESTDIR}${RC0}/K02xdm
	rm -f ${DESTDIR}${RC0}/K07athena
	ln -s ../init.d/athena ${DESTDIR}${RC0}/K07athena
	rm -f ${DESTDIR}${RC0}/K79athena.config
	ln -s ../init.d/athena.config ${DESTDIR}${RC0}/K79athena.config
	rm -f ${DESTDIR}${RC2}/S13athena.config
	ln -s ../init.d/athena.config ${DESTDIR}${RC2}/S13athena.config
	rm -f ${DESTDIR}${RC2}/S24autoconfig.reboot
	ln -s ../init.d/autoconfig.reboot ${DESTDIR}${RC2}/S24autoconfig.reboot
	rm -f ${DESTDIR}${RC2}/S36finish-update
	ln -s ../init.d/finish-update ${DESTDIR}${RC2}/S36finish-update
	rm -f ${DESTDIR}${RC2}/S93athena
	ln -s ../init.d/athena ${DESTDIR}${RC2}/S93athena
	rm -f ${DESTDIR}${RC2}/S98xdm
	ln -s ../init.d/axdm ${DESTDIR}${RC2}/S98xdm

clean:
	rm -f os-checkfiles.o array.o os-checkfiles

distclean:
	rm -f os-checkfiles.o array.o os-checkfiles
