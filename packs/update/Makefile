# $Id: Makefile,v 1.19 2003-04-22 05:32:26 ghudson Exp $

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
	install -c -m 0555 vers/athena-deadfiles-9.2 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/noask ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/irix52-53 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris24-251 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris251-26 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris26-pkgs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris26-ptchs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris26-7 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris7-hardlinks ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris7-local-sun4m ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris7-local-sun4u ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris7-nolocal-sun4m ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris7-nolocal-sun4u ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris27-8 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris27-pkgs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris27-ptchs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-8.2 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-local-sun4m ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-local-sun4u ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-nolocal-sun4m ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-nolocal-sun4u ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-patches.9.0.0 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8.2-local ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8.2-nolocal ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris8-patches.9.1.0 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris28-pkgs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris28-ptchs ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/solaris9.2 ${DESTDIR}${UPDATE}
	install -c -m 0555 vers/deadfiles-9.1.12 ${DESTDIR}${UPDATE}
	for i in vers/*.*.*; do \
		install -c -m 0555 $$i ${DESTDIR}${UPDATE}; \
	done

clean:
	rm -f upvers.o upvers
	cd os/${OS} && ${MAKE} $@

distclean:
	rm -f upvers.o upvers
	cd os/${OS} && ${MAKE} $@
