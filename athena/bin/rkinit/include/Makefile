#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v 1.2 1989-11-13 16:48:40 qjb Exp $
# $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v $
# $Author: qjb $
#

DESTDIR		=
INSTALLDIR 	= /usr/include

all: rkinit_err.h

rkinit_err.h: 
	chdir ../lib; make ../include/rkinit_err.h

depend: rkinit_err.h

install: all
	cp rkinit_err.h rkinit.h $(DESTDIR)$(INSTALLDIR)

clean:
	rm -f *~ rkinit_err.h
