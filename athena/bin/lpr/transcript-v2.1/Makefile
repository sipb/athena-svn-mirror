# transcript/src/Makefile.bsd
#
# Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# RCSID: $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/Makefile,v 1.2 1990-04-16 18:06:09 epeisach Exp $


LPROGS = pscomm lps40comm
BPROGS = 

OBJECTS = psbanner.o pscomm.o psrv.o pstext.o \
	enscript.o ps4014.o ps630.o pscat.o pscatmap.o \
	psdit.o psplot.o psrev.o \
	psutil.o

PSLIBDIR=/usr/athena/lib/ps
PSTEMPDIR=/tmp
DITDIR=/usr/lib/font
TROFFFONTDIR=$(PSLIBDIR)/troff.font

CFLAGS = -O -DBSD -DSYSLOG -DZEPHYR

programs all:	${LPROGS} ${BPROGS}

pscomm: pscomm.o psutil.o config.o
	cc ${CFLAGS} -o pscomm pscomm.o psutil.o config.o -lzephyr \
		-lcom_err -lkrb -ldes

lps40comm: lps40comm.o psutil.o config.o
	cc ${CFLAGS} -o lps40comm lps40comm.o psutil.o config.o


${OBJECTS}: transcript.h
pscomm.o psbanner.o pscat.o: psspool.h
pscat.o pscatmap.o: action.h 

config.c: config.proto config Makefile
	-rm -f - config.c
	sed	-e s,PSLIBDIR,$(PSLIBDIR),g \
		-e s,TROFFFONTDIR,$(TROFFFONTDIR),g \
		-e s,DITDIR,$(DITDIR),g \
		-e s,PSTEMPDIR,$(PSTEMPDIR),g \
		config.proto >config.c"

install: ${LPROGS} ${BPROGS}
	-cd $$PSLIBDIR; rm -f ${LPROGS} ${BPROGS}
	install -c -o root -g staff -s pscomm	$(DESTDIR)$(PSLIBDIR)/
	install -c -o root -g staff -s lps40comm	$(DESTDIR)$(PSLIBDIR)/
	install -c -m 755 -o root -g staff psif	$(DESTDIR)$(PSLIBDIR)/psint.sh

clean:
	rm -f - *.o config.c ${BPROGS} ${LPROGS} *BAK *CKP .emacs_[0-9]* *~

depend:
