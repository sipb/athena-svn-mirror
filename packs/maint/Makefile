# $Id: Makefile,v 1.17 2003-10-23 22:35:27 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
ATHMANDIR=/usr/athena/man
USRLIBDIR=/usr/lib

all: os-checkfiles
	cd os/${OS} && ${MAKE} $@

os-checkfiles: os-checkfiles.o array.o
	${CC} ${LDFLAGS} -o $@ os-checkfiles.o array.o

check:
	cd os/${OS} && ${MAKE} $@

install:
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${ATHMANDIR}/man8
	mkdir -p ${DESTDIR}${USRLIBDIR}
	install -c -m 0555 clean_tmp_areas.sh \
		${DESTDIR}${ATHRETCDIR}/clean_tmp_areas
	install -c -m 0555 config_afs.sh ${DESTDIR}${ATHRETCDIR}/config_afs
	install -c -m 0555 counterlog.sh ${DESTDIR}${ATHRETCDIR}/counterlog
	install -c -m 0555 local-menus.sh ${DESTDIR}${ATHRETCDIR}/local-menus
	install -c -m 0555 local-lockers.sh \
		${DESTDIR}${ATHRETCDIR}/local-lockers
	install -c -m 0555 netparams.sh ${DESTDIR}${ATHRETCDIR}/netparams
	install -c -m 0555 reactivate.sh ${DESTDIR}${ATHRETCDIR}/reactivate
	install -c -m 0444 reactivate.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 save_cluster_info.sh \
		${DESTDIR}${ATHRETCDIR}/save_cluster_info
	install -c -m 0444 save_cluster_info.8 ${DESTDIR}${ATHMANDIR}/man8
	install -c -m 0555 sendmail-relay-select ${DESTDIR}${ATHRETCDIR}
	install -c -m 0555 shutdown_notify.sh \
		${DESTDIR}${ATHRETCDIR}/shutdown_notify
	install -c -m 0444 syncconf.8 ${DESTDIR}${ATHMANDIR}/man8
	cd os/${OS} && ${MAKE} $@

clean:
	rm -f os-checkfiles os-checkfiles.o array.o
	cd os/${OS} && ${MAKE} $@

distclean:
	cd os/${OS} && ${MAKE} $@
