# $Id: Makefile,v 1.3 1997-04-01 19:16:40 ghudson Exp $

SHELL=/bin/sh
ATHMANDIR=/usr/athena/man
USRETC=/usr/etc

all:

check:

install:
	-mkdir -p ${DESTDIR}${USRETC}
	install -c -m 555 -o 0 -g 0 snmpd ${DESTDIR}${USRETC}
	cd ${DESTDIR}${ATHMANDIR} && /usr/lib/makewhatis -M . whatis
	cd ${DESTDIR}${ATHMANDIR} && for n in 1 2 3 4 5 6 7 8; do \
		if [ ! -f man$$n/* ]; then \
			continue; \
		fi; \
		for file in man$$n/*; do \
			base=`basename $$file`; \
			if [ -l $$file ]; then \
				continue; \
			fi; \
			rm -f cat$$n/$$base; \
			set foo; set -- `head -1 $$file`; \
			if [ $$# -eq 2 -a "$$1" = .so ]; then \
				ln -s ../$$2 cat$$n/$$base; \
			else \
				nroff -man $$file > cat$$n/$$base; \
				chmod 444 cat$$n/$$base; \
			fi; \
			chown -h 0.0 cat$$n/$$base; \
		done \
	done

clean:

distclean:
