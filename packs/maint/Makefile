# $Id: Makefile,v 1.1 1996-09-20 05:02:16 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
ATMANDIR=/usr/athena/man

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 clean_tmp_areas.sh \
		${DESTDIR}${ATHRETCDIR}/clean_tmp_areas
	install -c -m 0555 config_afs.sh ${DESTDIR}${ATHRETCDIR}/config_afs
	install -c -m 0444 reactivate.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 reactivate.sh ${DESTDIR}${ATHRETCDIR}/reactivate
	install -c -m 0555 shutdown_notify.sh \
		${DESTDIR}${ATHRECTDIR}/shutdown_notify.sh
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
