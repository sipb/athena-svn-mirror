# $Id: Makefile,v 1.10 2000-06-08 18:48:18 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
ATHMANDIR=/usr/athena/man

all:
	cd os/${OS} && ${MAKE} $@

check:
	cd os/${OS} && ${MAKE} $@

install:
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 clean_tmp_areas.sh \
		${DESTDIR}${ATHRETCDIR}/clean_tmp_areas
	install -c -m 0555 config_afs.sh ${DESTDIR}${ATHRETCDIR}/config_afs
	install -c -m 0555 local-netscape.sh \
		${DESTDIR}${ATHRETCDIR}/local-netscape
	install -c -m 0555 netparams.sh ${DESTDIR}${ATHRETCDIR}/netparams
	install -c -m 0555 reactivate.sh ${DESTDIR}${ATHRETCDIR}/reactivate
	install -c -m 0444 reactivate.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 save_cluster_info.sh \
		${DESTDIR}${ATHRETCDIR}/save_cluster_info
	install -c -m 0444 save_cluster_info.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 shutdown_notify.sh \
		${DESTDIR}${ATHRETCDIR}/shutdown_notify
	install -c -m 0444 syncconf.8 ${DESTDIR}${ATHMANDIR}/man8
	cd os/${OS} && ${MAKE} $@

clean:
	cd os/${OS} && ${MAKE} $@

distclean:
	cd os/${OS} && ${MAKE} $@
