#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/Makefile,v 1.4 1990-07-17 13:56:55 qjb Exp $
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
	install -c -m 444 rkinit.h $(DESTDIR)$(INSTALLDIR)/rkinit.h
	install -c -m 444 rkinit_err.h $(DESTDIR)$(INSTALLDIR)/rkinit_err.h

clean:
	rm -f *~ rkinit_err.h
