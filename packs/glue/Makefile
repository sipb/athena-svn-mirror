# $Id: Makefile,v 1.2 1996-09-25 17:50:49 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man
ATTACHRUN=addusr blanche chfn chpobox chsh kermit mkserv mailmaint moira \
	mrcheck namespace olc_answers olh sis techinfo userreg \
	xinfo xtechinfo

all: ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@

${ATTACHRUN}: specs template.sh
	set `grep '^$@' specs`; locker=$$2; program=$${3-$$1}; \
		path=`athdir -c -p /mit/$$2 -t bin`/$$program; \
		sed -e "s|@LOCKER@|$$2|g" -e "s|@PATH@|$$path|g" \
			-e "s|@PROGRAM@|$$program|g" template.sh > $@

verify-message: verify-message.in
	path=`athdir -c -p /mit/pgp -t bin`; \
		sed -e 's|@PATH@|$$path|g' verify-message.in > $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 addusr ${DESTDIR}${ATHBINDIR}
	install -c -m 555 blanche ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chfn ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chpobox ${DESTDIR}${ATHBINDIR}
	install -c -m 555 chsh ${DESTDIR}${ATHBINDIR}
	install -c -m 555 kermit ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mkserv ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mailmaint ${DESTDIR}${ATHBINDIR}
	install -c -m 555 moira ${DESTDIR}${ATHBINDIR}
	install -c -m 555 mrcheck ${DESTDIR}${ATHBINDIR}
	install -c -m 555 namespace ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olc_answers ${DESTDIR}${ATHBINDIR}
	install -c -m 555 olh ${DESTDIR}${ATHBINDIR}
	install -c -m 555 psgrind.sh ${DESTDIR}${ATHBINDIR}/psgrind
	install -c -m 555 sis ${DESTDIR}${ATHBINDIR}
	install -c -m 555 techinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 555 userreg ${DESTDIR}${ATHBINDIR}
	install -c -m 555 verify-message ${DESTDIR}${ATHBINDIR}
	install -c -m 555 xinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 555 xtechinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 444 sis.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 444 xinfo.1 ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 444 xtechinfo.1 ${DESTDIR}${ATHMANDIR}/man1
	rm -f ${DESTDIR}${ATHBINDIR}/help
	ln -s olh ${DESTDIR}${ATHBINDIR}/help
	cd platform/${HOSTTYPE} && ${MAKE} $@

clean:
	rm -f ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	rm -f ${ATTACHRUN} verify-message
	cd platform/${HOSTTYPE} && ${MAKE} $@
