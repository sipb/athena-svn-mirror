# $Id: Makefile,v 1.15 2003-10-26 20:12:28 ghudson Exp $

ATHRETCDIR=/etc/athena
ATHETCDIR=/usr/athena/etc
ATHLOGINDIR=${ATHRETCDIR}/login
INITD=/etc/init.d
RC0=/etc/rc0.d
RC1=/etc/rc1.d
RC2=/etc/rc2.d
RCS=/etc/rcS.d

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
	install -c -m 0755 Xreset ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 Xstartup ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 athena-bind.rc ${DESTDIR}${INITD}/athena-bind
	install -c -m 0755 athena-inetd.rc ${DESTDIR}${INITD}/athena-inetd
	install -c -m 0755 athena-lpd.rc ${DESTDIR}${INITD}/athena-lpd
	install -c -m 0755 athena-ntp.rc ${DESTDIR}${INITD}/athena-ntp
	install -c -m 0755 athena-olc.rc ${DESTDIR}${INITD}/athena-olc
	install -c -m 0755 athena-opensshd.rc \
		${DESTDIR}${INITD}/athena-opensshd
	install -c -m 0755 athena-sendmail.rc \
		${DESTDIR}${INITD}/athena-sendmail
	install -c -m 0755 athena-update_server.rc \
		${DESTDIR}${INITD}/athena-update_server
	install -c -m 0755 athena-ws.rc ${DESTDIR}${INITD}/athena-ws
	install -c -m 0755 dhcp.rename ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 nfs.client ${DESTDIR}${INITD}
	install -c -m 0755 rpc ${DESTDIR}${INITD}
	install -c -m 0755 inetsvc ${DESTDIR}${INITD}
	install -c -m 0755 oscheck.sh ${DESTDIR}${ATHETCDIR}/oscheck
	install -c -m 0555 sendmail.sh ${DESTDIR}${USRLIBDIR}/sendmail
	install -c -m 0755 syncconf.rc ${DESTDIR}${INITD}/syncconf
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	install -c -m 0755 xwrapper.sh ${DESTDIR}${ATHRETCDIR}/xwrapper
	install -c -m 0755 zephyrd.rc ${DESTDIR}${INITD}/zephyrd
	install -c -m 0755 zhm.rc ${DESTDIR}${INITD}/zhm
	install -c -m 0755 ../../os-checkfiles ${DESTDIR}${ATHETCDIR}
	install -c -m 0555 ../../sendmail-relay-select ${DESTDIR}${ATHRETCDIR}
	rm -f ${DESTDIR}${RC0}/K90athena-ws
	ln -s ../init.d/athena-ws ${DESTDIR}${RC0}/K90athena-ws
	rm -f ${DESTDIR}${RC0}/K91athena-inetd
	ln -s ../init.d/athena-inetd ${DESTDIR}${RC0}/K91athena-inetd
	rm -f ${DESTDIR}${RC0}/K91athena-lpd
	ln -s ../init.d/athena-lpd ${DESTDIR}${RC0}/K91athena-lpd
	rm -f ${DESTDIR}${RC0}/K91athena-ntp
	ln -s ../init.d/athena-ntp ${DESTDIR}${RC0}/K91athena-ntp
	rm -f ${DESTDIR}${RC0}/K91athena-olc
	ln -s ../init.d/athena-olc ${DESTDIR}${RC0}/K91athena-olc
	rm -f ${DESTDIR}${RC0}/K91athena-opensshd
	ln -s ../init.d/athena-opensshd ${DESTDIR}${RC0}/K91athena-opensshd
	rm -f ${DESTDIR}${RC0}/K91athena-sendmail
	ln -s ../init.d/athena-sendmail ${DESTDIR}${RC0}/K91athena-sendmail
	rm -f ${DESTDIR}${RC0}/K91athena-update_server
	ln -s ../init.d/athena-update_server \
		${DESTDIR}${RC0}/K91athena-update_server
	rm -f ${DESTDIR}${RC0}/K91zephyrd
	ln -s ../init.d/zephyrd ${DESTDIR}${RC0}/K91zephyrd
	rm -f ${DESTDIR}${RC0}/K91zhm
	ln -s ../init.d/zhm ${DESTDIR}${RC0}/K91zhm
	rm -f ${DESTDIR}${RC0}/K95syncconf
	ln -s ../init.d/syncconf ${DESTDIR}${RC0}/K95syncconf
	rm -f ${DESTDIR}${RC2}/S13syncconf
	ln -s ../init.d/syncconf ${DESTDIR}${RC2}/S13syncconf
	rm -f ${DESTDIR}${RC2}/S21savecore
	ln -s ../init.d/savecore ${DESTDIR}${RC2}/S21savecore
	rm -f ${DESTDIR}${RC2}/S72athena-bind
	ln -s ../init.d/athena-bind ${DESTDIR}${RC2}/S72athena-bind
	rm -f ${DESTDIR}${RC2}/S72dhcp.rename
	ln -s ../init.d/dhcp.rename ${DESTDIR}${RC2}/S72dhcp.rename
	rm -f ${DESTDIR}${RC2}/S70finish-update
	ln -s ../init.d/finish-update ${DESTDIR}${RC2}/S70finish-update
	rm -f ${DESTDIR}${RC2}/S90athena-ws
	ln -s ../init.d/athena-ws ${DESTDIR}${RC2}/S90athena-ws
	rm -f ${DESTDIR}${RC2}/S91athena-inetd
	ln -s ../init.d/athena-inetd ${DESTDIR}${RC2}/S91athena-inetd
	rm -f ${DESTDIR}${RC2}/S91athena-lpd
	ln -s ../init.d/athena-lpd ${DESTDIR}${RC2}/S91athena-lpd
	rm -f ${DESTDIR}${RC2}/S91athena-ntp
	ln -s ../init.d/athena-ntp ${DESTDIR}${RC2}/S91athena-ntp
	rm -f ${DESTDIR}${RC2}/S91athena-olc
	ln -s ../init.d/athena-olc ${DESTDIR}${RC2}/S91athena-olc
	rm -f ${DESTDIR}${RC2}/S91athena-opensshd
	ln -s ../init.d/athena-opensshd ${DESTDIR}${RC2}/S91athena-opensshd
	rm -f ${DESTDIR}${RC2}/S91athena-sendmail
	ln -s ../init.d/athena-sendmail ${DESTDIR}${RC2}/S91athena-sendmail
	rm -f ${DESTDIR}${RC2}/S91athena-update_server
	ln -s ../init.d/athena-update_server \
		${DESTDIR}${RC2}/S91athena-update_server
	rm -f ${DESTDIR}${RC2}/S91zephyrd
	ln -s ../init.d/zephyrd ${DESTDIR}${RC2}/S91zephyrd
	rm -f ${DESTDIR}${RC2}/S91zhm
	ln -s ../init.d/zhm ${DESTDIR}${RC2}/S91zhm
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
