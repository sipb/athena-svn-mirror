# $Id: Makefile,v 1.2 1997-10-15 23:31:07 ghudson Exp $

SHELL=/bin/sh
SUBDIRS=man arch/${ATHENA_SYS}

all:
	for i in ${SUBDIRS}; do (cd $$i && ${MAKE} $@) || exit 1; done

check install clean distclean:
	for i in ${SUBDIRS}; do (cd $$i && ${MAKE} $@) || exit 1; done
