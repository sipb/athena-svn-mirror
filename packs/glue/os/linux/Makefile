# $Id: Makefile,v 1.1.4.1 2000-02-25 20:33:07 tb Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATTACHRUN=lastlog
ATTRUNTEMPLATE=../../template.sh

all: ${ATTACHRUN}

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lastlog ${DESTDIR}${ATHBINDIR}

clean:

distclean:

lastlog: ${ATTRUNTEMPLATE}
	sed -e "s|@LOCKER@|consult|g" -e "s|@PROGRAM@|lastlog|g" \
		${ATTRUNTEMPLATE} > $@
