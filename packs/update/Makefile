# $Id: Makefile,v 1.10 2000-02-18 15:08:25 ghudson Exp $

SHELL=/bin/sh
ATHRETCDIR=/etc/athena
UPDATE=/usr/athena/lib/update
CFLAGS=-g

all: upvers
	cd os/${OS} && ${MAKE} $@

upvers: upvers.o
	${CC} ${LDFLAGS} -o upvers upvers.o ${LIBS}

check:
	cd os/${OS} && ${MAKE} $@

# We're going to cheat and rely on the state of the source tree to find version
# scripts in vers; the alternative is editing this Makefile for every patch
# release, which would make it very hard to track actual changes to this file.
install:
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${UPDATE}
	install -c -m 0555 do-update.sh ${DESTDIR}${UPDATE}/do-update
	install -c -m 0555 finish-update.sh ${DESTDIR}${UPDATE}/finish-update
	install -c -m 0555 update-environment.sh \
				${DESTDIR}${UPDATE}/update-environment
	install -c -m 0555 update_ws.sh ${DESTDIR}/update_ws
	install -c -m 0555 upvers ${DESTDIR}${UPDATE}
	rm -f ${DESTDIR}/auto_update
	ln -s update_ws ${DESTDIR}/auto_update
	rm -f ${DESTDIR}${ATHRETCDIR}/update_ws
	ln -s /srvd/update_ws ${DESTDIR}${ATHRETCDIR}/update_ws
	cd os/${OS} && ${MAKE} $@
	install -c -m 0555 vers/irix52-53 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris24-251 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris251-26 ${DESTDIR}${UPDATE}
	for i in vers/*.*.*; do \
		install -c -m 0555 $$i ${DESTDIR}${UPDATE}; \
	done

clean:
	rm -f upvers.o upvers
	cd os/${OS} && ${MAKE} $@

distclean:
	rm -f upvers.o upvers
	cd os/${OS} && ${MAKE} $@
