# $Id: Makefile,v 1.5 1997-09-24 07:15:07 ghudson Exp $

SHELL=/bin/sh
ATHMANDIR=/usr/athena/man
WHATIS=${ATHMANDIR}/whatis
USRETC=/usr/etc

all:

check:

install:
	-mkdir -p ${DESTDIR}${USRETC}
	install -c -m 555 snmpd ${DESTDIR}${USRETC}
	cd ${DESTDIR}${ATHMANDIR} && for manfile in man*/*; do \
		n=`echo "$$manfile" | sed -e 's/man\(.\).*/\1/'`; \
		catfile=cat$$n/`basename $$manfile`; \
		if [ -l "$$manfile" ]; then \
			continue; \
		fi; \
		if [ -f $$catfile ]; then \
			if [ "`find $$catfile -newer $$manfile -print`" \
			     != "" ]; then \
				continue; \
			fi; \
		fi; \
		rm -f $$catfile; \
		set foo; set -- `head -1 $$manfile`; \
		if [ $$# -eq 2 -a "$$1" = .so ]; then \
			ln -s ../$$2 $$catfile; \
			continue; \
		fi; \
		sed -e 's,^\(\.so  *\)\(/usr/athena\),\1${ATHTOOLROOT}\2,' \
			$$manfile | nroff -man > $$catfile; \
		chmod 444 "$$catfile"; \
	done
	outofdate=false; \
	if [ ! -f ${DESTDIR}${WHATIS} ]; then \
		outofdate=true; \
	elif [ "`find ${DESTDIR}${ATHMANDIR}/man* -newer ${DESTDIR}${WHATIS} \
		-print`" != "" ]; then \
		outofdate=true; \
	fi; \
	if [ "$$outofdate" = true ]; then \
		rm -f ${DESTDIR}${WHATIS}; \
		for i in ${DESTDIR}${ATHMANDIR}/cat*/*; do \
			sed -n -f makewhatis.sed "$$i"; \
		done | sort -u > ${DESTDIR}${WHATIS}; \
		chmod 444 ${DESTDIR}${WHATIS}; \
	fi

clean:

distclean:
