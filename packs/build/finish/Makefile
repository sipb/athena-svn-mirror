# $Id: Makefile,v 1.1 1996-09-20 05:14:15 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
UPDATE=${ATHLIBDIR}/update

all: fix_owners rvdinfo
	cd platform/${HOSTTYPE} && ${MAKE} $@

fix_owners: fix_owners.o
	${CC} -o $@ fix_owners.o

rvdinfo: ../version
	. ../version; \
		machine=`echo "$$HOSTTYPE" | tr "[a-z]" "[A-Z]"`; \
		version="$$major.$$minor.$$patch"; \
		echo "Athena RVD ($$machine) Version $$version `date`" > $@

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
	cd platform/${HOSTTYPE} && ${MAKE} $@

distclean:
	cd platform/${HOSTTYPE} && ${MAKE} $@
