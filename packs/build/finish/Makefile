# $Id: Makefile,v 1.10 2001-04-12 17:14:53 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
UPDATE=${ATHLIBDIR}/update
CFLAGS=-g

all: fix_owners rvdinfo
	cd os/${OS} && ${MAKE} $@

fix_owners: fix_owners.o
	${CC} -o $@ fix_owners.o

rvdinfo:
	version="$$ATHENA_MAJOR_VERSION.$$ATHENA_MINOR_VERSION"; \
	version="$$version.$$ATHENA_PATCH_VERSION"; \
	echo "Athena RVD ($$ATHENA_HOSTTYPE) Version $$version `date`" > $@

check:
	cd os/${OS} && ${MAKE} $@

# We generate FILES at install time, which is a violation of the normal
# build system rules.  It needs to wait until after everything is put
# in place on the packs, including the other things we install.
install:
	@if [ -r ${DESTDIR}/.rvdinfo ]; then \
		dvers=`sed -e 's/^.* \([0-9]*\.[0-9]*\)\..*/\1/' \
			${DESTDIR}/.rvdinfo`; \
		vers=$$ATHENA_MAJOR_VERSION.$$ATHENA_MINOR_VERSION; \
		if [ "$$dvers" != "$$vers" ]; then \
			echo "Error: major or minor version mismatch."; \
			exit 1; \
		fi; \
	fi
	cd os/${OS} && ${MAKE} $@
	mkdir -p ${DESTDIR}${ATHLIBDIR}/stats
	rm -f ${DESTDIR}/patch
	vers="$$ATHENA_MAJOR_VERSION.$$ATHENA_MINOR_VERSION"; \
		ln -s /afs/athena.mit.edu/system/patch-$$vers/${ATHENA_SYS} \
		 ${DESTDIR}/patch
	install -c -m 444 rvdinfo ${DESTDIR}/.rvdinfo
	./fix_owners ${DESTDIR}
	chmod go-w ${DESTDIR}
	${DESTDIR}${ATHETCDIR}/track -w -F${DESTDIR} -W${DESTDIR}${ATHLIBDIR} \
		-s stats/sys_rvd slists/sys_rvd
	${DESTDIR}${ATHETCDIR}/track -w -F${DESTDIR} -W${DESTDIR}${ATHLIBDIR} \
		-s stats/sys_rvd.local slists/sys_rvd.local
	chown 0 ${DESTDIR}${ATHLIBDIR}/stats/*
	chgrp 0 ${DESTDIR}${ATHLIBDIR}/stats/*
	(cd ${DESTDIR} && find . -print | sort) > FILES
	install -c -m 444 -o 0 -g 0 FILES ${DESTDIR}${UPDATE}/FILES

clean:
	rm -f fix_owners.o fix_owners rvdinfo FILES
	cd os/${OS} && ${MAKE} $@

distclean:
	rm -f fix_owners.o fix_owners rvdinfo FILES
	cd os/${OS} && ${MAKE} $@
