# $Id: Makefile,v 1.2 1996-12-27 21:38:28 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
UPDATE=${ATHLIBDIR}/update
CFLAGS=-g

all: fix_owners rvdinfo
	cd platform/${HOSTTYPE} && ${MAKE} $@

fix_owners: fix_owners.o
	${CC} -o $@ fix_owners.o

rvdinfo: ../version
	. ../version; \
		version="$$major.$$minor.$$patch"; \
		echo "Athena RVD ($$HOSTTYPE) Version $$version `date`" > $@

check:
	cd platform/${HOSTTYPE} && ${MAKE} $@

# We generate FILES at install time, which is a violation of the normal
# build system rules.  It needs to wait until after everything is put
# in place on the packs, including the other things we install.
install:
	rm -f ${DESTDIR}/patch
	. ../version; \
		vers="$$major.$$minor"; \
		ln -s /afs/athena.mit.edu/system/patch-$$vers/${ATHENA_SYS} \
			 ${DESTDIR}/patch
	install -c -m 444 rvdinfo ${DESTDIR}/.rvdinfo
	-mkdir -p ${DESTDIR}${ATHLIBDIR}/stats
	./fix_owners ${DESTDIR}
	cd platform/${HOSTTYPE} && ${MAKE} $@
	${DESTDIR}${ATHETCDIR}/track -w -F${DESTDIR} -W${DESTDIR}${ATHLIBDIR} \
		-s stats/sys_rvd slists/sys_rvd
	chown 0 ${DESTDIR}${ATHLIBDIR}/stats/*
	chgrp 0 ${DESTDIR}${ATHLIBDIR}/stats/*
	(cd ${DESTDIR} && find . -print | sort) > FILES
	install -c -m 444 -o 0 -g 0 FILES ${DESTDIR}${UPDATE}/FILES

clean:
	rm -f fix_owners.o fix_owners rvdinfo FILES
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	rm -f fix_owners.o fix_owners rvdinfo FILES
	cd platform/${HOSTTYPE} && ${MAKE} $@
