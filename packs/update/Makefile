# $Id: Makefile,v 1.2 1996-10-10 18:31:32 ghudson Exp $

SHELL=/bin/sh
UPDATE=/usr/athena/lib/update
CFLAGS=${CDEBUGFLAGS}
CDEBUGFLAGS=-O

all: upvers
	cd platform/${HOSTTYPE} && ${MAKE} $@

upvers: upvers.o
	${CC} ${LDFLAGS} -o upvers upvers.o ${LIBS}

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

# We're going to cheat and rely on the state of the source tree to find version
# scripts in vers; the alternative is editing this Makefile for every patch
# release, which would make it very hard to track actual changes to this file.
install:
	-mkdir -p ${DESTDIR}${UPDATE}
	install -c -m 0555 do_update.sh ${DESTDIR}${UPDATE}/do_update
	install -c -m 0555 update_ws.sh ${DESTDIR}/update_ws
	install -c -s -m 0555 upvers ${DESTDIR}${UPDATE}
	rm -f ${DESTDIR}/auto_update
	ln -s update_ws ${DESTDIR}/auto_update
	cd platform/${HOSTTYPE} && ${MAKE} $@
	for i in vers/?.??; do \
		install -c -m 0444 $$i ${DESTDIR}${UPDATE}; \
	done

clean:
	rm -f upvers.o upvers
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	rm -f upvers.o upvers
	cd platform/${HOSTTYPE} && ${MAKE} $@
