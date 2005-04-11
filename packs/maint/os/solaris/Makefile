# $Id: Makefile,v 1.21 2005-04-11 14:11:13 rbasch Exp $

ATHRETCDIR=/etc/athena
ATHETCDIR=/usr/athena/etc
ATHLOGINDIR=${ATHRETCDIR}/login
INITD=/etc/init.d
RC0=/etc/rc0.d
RC1=/etc/rc1.d
RC2=/etc/rc2.d
RCS=/etc/rcS.d
SVCMETHOD=/lib/svc/method
USRLIBDIR=/usr/lib

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHETCDIR}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHLOGINDIR}
	mkdir -p ${DESTDIR}${INITD}
	mkdir -p ${DESTDIR}${RC0}
	mkdir -p ${DESTDIR}${RC1}
	mkdir -p ${DESTDIR}${RC2}
	mkdir -p ${DESTDIR}${RCS}
	mkdir -p ${DESTDIR}${SVCMETHOD}
	mkdir -p ${DESTDIR}${USRLIBDIR}
	install -c -m 0755 Xreset ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 Xstartup ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 athena-ws.rc ${DESTDIR}${INITD}/athena-ws
	install -c -m 0755 console-login.sh \
		${DESTDIR}${SVCMETHOD}/console-login
	install -c -m 0755 dhcp.rename ${DESTDIR}${INITD}
	install -c -m 0755 oscheck.sh ${DESTDIR}${ATHETCDIR}/oscheck
	install -c -m 0755 svc-named.sh ${DESTDIR}${ATHRETCDIR}/svc-named
	install -c -m 0755 syncconf.rc ${DESTDIR}${INITD}/syncconf
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	install -c -m 0755 verify.sh ${DESTDIR}${ATHRETCDIR}/verify
	install -c -m 0555 verify-pkgs.sh ${DESTDIR}${ATHETCDIR}/verify-pkgs
	install -c -m 0755 ../../os-checkfiles ${DESTDIR}${ATHETCDIR}
	rm -f ${DESTDIR}${RC0}/K90athena-ws
	ln -s ../init.d/athena-ws ${DESTDIR}${RC0}/K90athena-ws
	rm -f ${DESTDIR}${RC0}/K95syncconf
	ln -s ../init.d/syncconf ${DESTDIR}${RC0}/K95syncconf
	rm -f ${DESTDIR}${RC2}/S13syncconf
	ln -s ../init.d/syncconf ${DESTDIR}${RC2}/S13syncconf
	rm -f ${DESTDIR}${RC2}/S21savecore
	ln -s ../init.d/savecore ${DESTDIR}${RC2}/S21savecore
	rm -f ${DESTDIR}${RC2}/S72dhcp.rename
	ln -s ../init.d/dhcp.rename ${DESTDIR}${RC2}/S72dhcp.rename
	rm -f ${DESTDIR}${RC2}/S90athena-ws
	ln -s ../init.d/athena-ws ${DESTDIR}${RC2}/S90athena-ws
clean:

distclean:
