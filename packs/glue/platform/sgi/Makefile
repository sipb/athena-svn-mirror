# $Id: Makefile,v 1.5.2.2 1998-03-09 22:46:24 ghudson Exp $

NSBINDIR=/usr/bin/X11
ATTACHRUN=netscape
ATTRUNTEMPLATE=../../template.sh

SHELL=/bin/sh

all: ${ATTACHRUN}

${ATTACHRUN}: specs ${ATTRUNTEMPLATE}
	set `grep '^$@[ 	]' specs`; locker=$$2; program=$${3-$$1}; \
		bindir=`athdir -c -p /mit/$$2 -t bin`; \
		sed -e "s|@LOCKER@|$$2|g" -e "s|@BINDIR@|$$bindir|g" \
			-e "s|@PROGRAM@|$$program|g" ${ATTRUNTEMPLATE} > $@

check:

install:
	-mkdir -p ${DESTDIR}${NSBINDIR}
	install -c -m 555 netscape ${DESTDIR}${NSBINDIR}

clean:
	rm -f ${ATTACHRUN}

distclean: clean
