MANDIR=/usr/man
MAN1=${DESTDIR}${MANDIR}/man1
MAN5=${DESTDIR}${MANDIR}/man5
MAN8=${DESTDIR}${MANDIR}/man8
all depend Makefile:

clean:
	rm -f *~

install:
	install -c -m 644 lpquota.1 ${MAN1}/
	install -c -m 644 lpc.8 ${MAN8}/
	install -c -m 644 lpq.1 ${MAN1}/
	install -c -m 644 lprm.1 ${MAN1}/
	install -c -m 644 lpr.1 ${MAN1}/
	install -c -m 644 lptest.1 ${MAN1}/
	install -c -m 644 printcap.5 ${MAN5}/
	install -c -m 644 lpd.8 ${MAN8}/
