#
# Makefile for turnin and related clients; Athena release
#
# $Author: epeisach $
# $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/Makefile,v $
# $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/Makefile,v 1.1 1991-02-13 10:00:28 epeisach Exp $
#
# Copyright (c) 1990, Massachusetts Institute of Technology
#

DIRS = rpc3.9 ets protocol lib clients

all: ${DIRS}

${DIRS}: always_remake
	CC=`sh -c "if [ $(MACHINE)x = vaxx ]; then echo gcc; else echo $(CC); fi"`; export CC; (cd $@; make all)

depend:
	@echo "(Make depend does nothing.  These programs don't need it.)"

clean:
	for i in ${DIRS} include; do \
		(cd $$i; make clean); \
	done

install:
	CC=`sh -c "if [ $(MACHINE)x = vaxx ]; then echo gcc; else echo $(CC); fi"`; export CC; (cd clients; make DESTDIR=$(DESTDIR) install)
	cd man; make DESTDIR=$(DESTDIR) install

always_remake:
