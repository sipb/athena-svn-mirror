# $Id: Makefile,v 1.1 1996-09-20 05:38:03 ghudson Exp $

SHELL=/bin/sh
SUBDIRS=man arch/${ATHENA_SYS}

all:
	for i in ${SUBDIRS}; do (cd $$i; ${MAKE} $@) || exit 1; done

check install clean distclean:
	for i in ${SUBDIRS}; do (cd $$i; ${MAKE} $@) || exit 1; done
