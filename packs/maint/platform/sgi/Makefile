# $Id: Makefile,v 1.5 1997-02-11 19:25:54 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
INITD=/etc/init.d
RC0=/etc/rc0.d
RC2=/etc/rc2.d

all:

check:

install:
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${INITD}
	-mkdir -p ${DESTDIR}${RC0}
	-mkdir -p ${DESTDIR}${RC2}
	install -c -m 0755 athena ${DESTDIR}${INITD}
	install -c -m 0755 athena.config ${DESTDIR}${INITD}
	install -c -m 0755 axdm ${DESTDIR}${INITD}
	install -c -m 0755 autoconfig.reboot ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 network ${DESTDIR}${INITD}
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	rm -f ${DESTDIR}${RC0}/K02axdm
	ln -s ../init.d/axdm ${DESTDIR}${RC0}/K02axdm
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
	rm -f ${DESTDIR}${RC2}/S98axdm
	ln -s ../init.d/axdm ${DESTDIR}${RC2}/S98axdm

clean:

distclean:
