# $Id: Makefile,v 1.17 2004-04-02 22:48:00 rbasch Exp $

ATHRETCDIR=/etc/athena
ATHETCDIR=/usr/athena/etc
ATHLOGINDIR=${ATHRETCDIR}/login
INITD=/etc/init.d
RC0=/etc/rc0.d
RC1=/etc/rc1.d
RC2=/etc/rc2.d
RCS=/etc/rcS.d
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
	mkdir -p ${DESTDIR}${USRLIBDIR}
	install -c -m 0755 Xreset ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 Xstartup ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 athena-ws.rc ${DESTDIR}${INITD}/athena-ws
	install -c -m 0755 dhcp.rename ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 nfs.client ${DESTDIR}${INITD}
	install -c -m 0755 rpc ${DESTDIR}${INITD}
	install -c -m 0755 inetsvc ${DESTDIR}${INITD}
	install -c -m 0755 oscheck.sh ${DESTDIR}${ATHETCDIR}/oscheck
	install -c -m 0755 syncconf.rc ${DESTDIR}${INITD}/syncconf
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
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
	rm -f ${DESTDIR}${RC2}/S70finish-update
	ln -s ../init.d/finish-update ${DESTDIR}${RC2}/S70finish-update
	rm -f ${DESTDIR}${RC2}/S90athena-ws
	ln -s ../init.d/athena-ws ${DESTDIR}${RC2}/S90athena-ws
	rm -f ${DESTDIR}${RCS}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RCS}/K42inetsvc
	rm -f ${DESTDIR}${RC0}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC0}/K42inetsvc
	rm -f ${DESTDIR}${RC1}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC1}/K42inetsvc
	rm -f ${DESTDIR}${RC2}/S72inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC2}/S72inetsvc
	rm -f ${DESTDIR}${RCS}/K41rpc
	ln -s ../init.d/rpc ${DESTDIR}${RCS}/K41rpc
	rm -f ${DESTDIR}${RC0}/K41rpc
	ln -s ../init.d/rpc ${DESTDIR}${RC0}/K41rpc
	rm -f ${DESTDIR}${RC1}/K41rpc
	ln -s ../init.d/rpc ${DESTDIR}${RC1}/K41rpc
	rm -f ${DESTDIR}${RC2}/S71rpc
	ln -s ../init.d/rpc ${DESTDIR}${RC2}/S71rpc
clean:

distclean:
