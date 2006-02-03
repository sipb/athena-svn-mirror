# $Id: Makefile,v 1.7 2006-02-03 16:21:31 rbasch Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin
ATTACHRUN=cc f77 f90 pc CC \
	funzip unzip unzipsfx zip zipcloak zipgrep zipinfo zipnote zipsplit \
	ps2epsi

ATTRUNTEMPLATE=../../template.sh

all: ${ATTACHRUN}

${ATTACHRUN}: specs ${ATTRUNTEMPLATE}
	set `grep '^$@[ 	]' specs`; locker=$$2; program=$${3-$$1}; \
		bindir=`athdir -c -p /mit/$$2 -t bin`; \
		sed -e "s|@LOCKER@|$$2|g" -e "s|@BINDIR@|$$bindir|g" \
			-e "s|@PROGRAM@|$$program|g" ${ATTRUNTEMPLATE} > $@

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${USRBINDIR}
	install -c -m 555 cc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f77 ${DESTDIR}${ATHBINDIR}
	install -c -m 555 f90 ${DESTDIR}${ATHBINDIR}
	install -c -m 555 pc ${DESTDIR}${ATHBINDIR}
	install -c -m 555 CC ${DESTDIR}${ATHBINDIR}
	install -c -m 555 funzip ${DESTDIR}${ATHBINDIR}
	install -c -m 555 unzip ${DESTDIR}${ATHBINDIR}
	install -c -m 555 unzipsfx ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zip ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zipcloak ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zipgrep ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zipinfo ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zipnote ${DESTDIR}${ATHBINDIR}
	install -c -m 555 zipsplit ${DESTDIR}${ATHBINDIR}
	install -c -m 555 ps2epsi ${DESTDIR}${ATHBINDIR}
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ${ATHBINDIR}/lp ${DESTDIR}${USRBINDIR}/lp
	rm -f ${DESTDIR}${ATHBINDIR}/otalk
	ln -s /usr/bin/talk ${DESTDIR}${ATHBINDIR}/otalk
	rm -f ${DESTDIR}${ATHBINDIR}/xlock.real
	ln -s /usr/openwin/bin/xlock ${DESTDIR}${ATHBINDIR}/xlock.real

clean:
	rm -f ${ATTACHRUN}

distclean: clean
