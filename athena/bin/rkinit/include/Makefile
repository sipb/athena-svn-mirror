#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v 1.1 1989-11-13 15:03:59 qjb Exp $
# $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v $
# $Author: qjb $
#

DESTDIR		=
INSTALLDIR 	= /usr/include

all: rkinit_err.h

rkinit_err.h: 
	chdir ../lib; make ../include/rkinit_err.h

depend:

install:
	cp rkinit_err.h rkinit.h $(DESTDIR)$(INSTALLDIR)

clean:
	rm -f *~ rkinit_err.h
