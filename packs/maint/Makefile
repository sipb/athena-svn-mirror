# $Id: Makefile,v 1.5 1998-12-03 19:36:36 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
ATHMANDIR=/usr/athena/man

all:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 clean_tmp_areas.sh \
		${DESTDIR}${ATHRETCDIR}/clean_tmp_areas
	install -c -m 0555 config_afs.sh ${DESTDIR}${ATHRETCDIR}/config_afs
	install -c -m 0555 netparams.sh ${DESTDIR}${ATHRETCDIR}/netparams
	install -c -m 0555 reactivate.sh ${DESTDIR}${ATHRETCDIR}/reactivate
	install -c -m 0444 reactivate.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 save_cluster_info.sh \
		${DESTDIR}${ATHRETCDIR}/save_cluster_info
	install -c -m 0444 save_cluster_info.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 shutdown_notify.sh \
		${DESTDIR}${ATHRETCDIR}/shutdown_notify
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

clean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
	cd arch/${ATHENA_SYS} && ${MAKE} $@
