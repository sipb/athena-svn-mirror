# $Id: Makefile,v 1.5 2000-03-16 15:09:25 miki Exp $

ATHRETCDIR=/etc/athena
INITD=/etc/init.d
RC0=/etc/rc0.d
RC2=/etc/rc2.d
RC1=/etc/rc1.d
RCS=/etc/rcS.d

all:

check:

install:
	mkdir -p ${DESTDIR}${INITD}
	mkdir -p ${DESTDIR}${RC0}
	mkdir -p ${DESTDIR}${RC2}
	install -c -m 0755 athena ${DESTDIR}${INITD}
	install -c -m 0755 athena.config ${DESTDIR}${INITD}
	install -c -m 0755 bind ${DESTDIR}${INITD}
	install -c -m 0755 dhcp.rename ${DESTDIR}${INITD}
	install -c -m 0755 finish-update ${DESTDIR}${INITD}
	install -c -m 0755 nfs.client ${DESTDIR}${INITD}
	install -c -m 0755 rpc ${DESTDIR}${INITD}
	install -c -m 0755 inetsvc ${DESTDIR}${INITD}
	install -c -m 0755 savecore ${DESTDIR}${INITD}
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	install -c -m 0755 xwrapper.sh ${DESTDIR}${ATHRETCDIR}/xwrapper
	rm -f ${DESTDIR}${RC0}/K90athena
	ln -s ../init.d/athena ${DESTDIR}${RC0}/K90athena
	rm -f ${DESTDIR}${RC0}/K95athena.config
	ln -s ../init.d/athena.config ${DESTDIR}${RC0}/K95athena.config
	rm -f ${DESTDIR}${RC2}/S13athena.config
	ln -s ../init.d/athena.config ${DESTDIR}${RC2}/S13athena.config
	rm -f ${DESTDIR}${RC2}/S21savecore
	ln -s ../init.d/savecore ${DESTDIR}${RC2}/S21savecore
	rm -f ${DESTDIR}${RC2}/S72bind
	ln -s ../init.d/bind ${DESTDIR}${RC2}/S72bind
	rm -f ${DESTDIR}${RC2}/S72dhcp.rename
	ln -s ../init.d/dhcp.rename ${DESTDIR}${RC2}/S72dhcp.rename
	rm -f ${DESTDIR}${RC2}/S70finish-update
	ln -s ../init.d/finish-update ${DESTDIR}${RC2}/S70finish-update
	rm -f ${DESTDIR}${RC2}/S90athena
	ln -s ../init.d/athena ${DESTDIR}${RC2}/S90athena
	rm -f ${DESTDIR}${RCS}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RCS}/K42inetsvc
	rm -f ${DESTDIR}${RC0}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC0}/K42inetsvc
	rm -f ${DESTDIR}${RC1}/K42inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC1}/K42inetsvc
	rm -f ${DESTDIR}${RC2}/S72inetsvc
	ln -s ../init.d/inetsvc ${DESTDIR}${RC2}/S72inetsvc
clean:

distclean:
