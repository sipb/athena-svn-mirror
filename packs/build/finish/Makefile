# $Id: Makefile,v 1.7 1999-12-15 05:27:19 jweiss Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
UPDATE=${ATHLIBDIR}/update
CFLAGS=-g

all: fix_owners rvdinfo
	cd os/${OS} && ${MAKE} $@

fix_owners: fix_owners.o
	${CC} -o $@ fix_owners.o

rvdinfo: ../version
	. ../version; \
		version="$$major.$$minor.$$patch"; \
		echo "Athena RVD ($$HOSTTYPE) Version $$version `date`" > $@

check:
	cd os/${OS} && ${MAKE} $@

# We generate FILES at install time, which is a violation of the normal
# build system rules.  It needs to wait until after everything is put
# in place on the packs, including the other things we install.
install:
	@if [ -r ${DESTDIR}/.rvdinfo ]; then \
		. ../version; \
		dvers=`sed -e 's/^.* \([0-9]*\.[0-9]*\)\..*/\1/' \
			${DESTDIR}/.rvdinfo`; \
		if [ "$$dvers" != "$$major.$$minor" ]; then \
			echo "Error: major or minor version mismatch."; \
			exit 1; \
		fi; \
	fi
	cd os/${OS} && ${MAKE} $@
	mkdir -p ${DESTDIR}${ATHLIBDIR}/stats
	rm -f ${DESTDIR}/patch
	. ../version; \
		vers="$$major.$$minor"; \
		ln -s /afs/athena.mit.edu/system/patch-$$vers/${ATHENA_SYS} \
			 ${DESTDIR}/patch
	install -c -m 444 rvdinfo ${DESTDIR}/.rvdinfo
	./fix_owners ${DESTDIR}
	chmod go-w ${DESTDIR}
	${DESTDIR}${ATHETCDIR}/track -w -F${DESTDIR} -W${DESTDIR}${ATHLIBDIR} \
		-s stats/sys_rvd slists/sys_rvd
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
