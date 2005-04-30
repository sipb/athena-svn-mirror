# $Id: Makefile,v 1.10 2005-04-30 16:05:27 rbasch Exp $

UPDATE=/usr/athena/lib/update

all:

check:

install:
	mkdir -p ${DESTDIR}${UPDATE}
	install -c -m 0444 admin-update ${DESTDIR}${UPDATE}
	install -c -m 0555 fix-partitions.sh ${DESTDIR}${UPDATE}/fix-partitions
	install -c -m 0555 pkg-order.pl ${DESTDIR}${UPDATE}/pkg-order
	install -c -m 0555 pkg-prune.sh ${DESTDIR}${UPDATE}/pkg-prune
	install -c -m 0555 pkg-update.sh ${DESTDIR}${UPDATE}/pkg-update
	install -c -m 0555 update-os.sh ${DESTDIR}${UPDATE}/update-os

clean:

distclean:
