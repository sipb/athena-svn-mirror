#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v 1.3 1990-07-17 12:56:27 qjb Exp $
# $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v $
# $Author: qjb $
#

DESTDIR		=
INSTALLDIR 	= /usr/include

all: rkinit_err.h

rkinit_err.h: 
	cd ../lib; make ../include/rkinit_err.h

depend: rkinit_err.h

install: all
	cp rkinit_err.h rkinit.h $(DESTDIR)$(INSTALLDIR)

clean:
	rm -f *~ rkinit_err.h
