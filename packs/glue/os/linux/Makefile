# $Id: Makefile,v 1.1.4.2 2000-03-13 18:18:45 tb Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATTACHRUN=lastlog
ATTRUNTEMPLATE=../../template.sh

all: ${ATTACHRUN}

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lastlog ${DESTDIR}${ATHBINDIR}
	rm -f ${DESTDIR}/usr/athena/bin/xlock

clean:

distclean:

lastlog: ${ATTRUNTEMPLATE}
	sed -e "s|@LOCKER@|consult|g" -e "s|@PROGRAM@|lastlog|g" \
		${ATTRUNTEMPLATE} > $@
