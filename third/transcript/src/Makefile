# transcript/src/Makefile.bsd
#
# Copyright (C) 1985,1987,1990,1991,1992 Adobe Systems Incorporated. All
# rights reserved. 
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/Makefile,v 1.2 1996-10-14 04:53:05 ghudson Exp $

SHELL = /bin/sh

CC = $$CCOMP

BINPROGS=enscript ps4014 ps630 pscat pscatmap psdit psplot pslpr psnup \
	afmdit psfax psdraft
# NOTE: psbanner and pscomm (from Transcript 2.1) are built and installed
#       from /source/athena/ucb/lpr/transcript-v2.1, so we don't do them here.
LIBPROGS=psdman map asc85ec lzwec
# LIBPROGS=psbanner pscomm psdman map asc85ec lzwec
PROGS=$(BINPROGS) $(LIBPROGS)

OBJECTS = psbanner.o pscomm.o psdman.o \
	enscript.o ps4014.o ps630.o pscat.o pscatmap.o \
	psdit.o psplot.o pslpr.o psnup.o \
	psutil.o psparse.o psfax.o psdraft.o

CFLAGS = $$CCFLAGS
LDFLAGS = ${CFLAGS}

programs: ${PROGS} 

afmdit: afmdit.o afmdit.h
	${CC} ${LDFLAGS} -o afmdit afmdit.o

asc85ec: asc85ec.o
	${CC} ${LDFLAGS} -o asc85ec asc85ec.o

lzwec: lzwec.o
	${CC} ${LDFLAGS} -o lzwec lzwec.o

psnup: psnup.o psutil.o
	${CC} ${LDFLAGS} -o psnup psutil.o  psnup.o -lm

pslpr: pslpr.o psparse.o psutil.o PSres.o compat.o
	${CC} ${LDFLAGS} -o pslpr pslpr.o psparse.o psutil.o PSres.o compat.o

enscript: enscript.o psutil.o PSres.o compat.o
	${CC} ${LDFLAGS} -o enscript enscript.o psutil.o PSres.o compat.o

pscat: pscat.o psutil.o 
	${CC} ${LDFLAGS} -o pscat pscat.o psutil.o 

psbanner: psbanner.o psutil.o
	${CC} ${LDFLAGS} -o psbanner psbanner.o psutil.o 

pscomm: pscomm.o psutil.o
	${CC} ${LDFLAGS} -o pscomm pscomm.o psutil.o ${XOBJS}

psdman: psdman.o psutil.o  psparse.o PSres.o compat.o
	${CC} ${LDFLAGS} -o psdman psdman.o psutil.o  psparse.o PSres.o compat.o

psplot: psplot.o psutil.o 
	${CC} ${LDFLAGS} -o psplot psplot.o psutil.o

ps630: ps630.o psutil.o
	${CC} ${LDFLAGS} -o ps630 ps630.o psutil.o

ps4014: ps4014.o psutil.o
	${CC} ${LDFLAGS} -o ps4014 ps4014.o psutil.o -lm

pscatmap: pscatmap.o psutil.o PSres.o
	${CC} ${LDFLAGS} -o pscatmap pscatmap.o psutil.o PSres.o

psdit: psdit.o psutil.o 
	${CC} ${LDFLAGS} -o psdit psdit.o psutil.o  

map: map.o PSres.o
	${CC} ${LDFLAGS} -o map map.o PSres.o

psfax: psfax.o
	${CC} ${LDFLAGS} -o psfax psfax.o

psdraft: psdraft.o psutil.o
	${CC} ${LDFLAGS} -o psdraft psdraft.o psutil.o


${OBJECTS}: transcript.h config.h compat.h
pscomm.o psbanner.o pscat.o: psspool.h
pscat.o pscatmap.o: action.h 
lzwec.o: lzw.h
pslpr.o psdman.o enscript.o PSres.o: PSres.h
compat.o: compat.c


### additions for Athena installation

INSTALL=../install-sh -c
INSTALL_EXEC=$(INSTALL) -m 755
STRIP=strip
RM=rm -f
DBINDIR=/srvd/this/needs/to/be/passed/in/when/make/is/invoked
DLIBDIR=/srvd/this/needs/to/be/passed/in/when/make/is/invoked

install:
	for i in ${BINPROGS}; do ${INSTALL_EXEC} $$i ${DBINDIR}; done
	for i in ${LIBPROGS}; do ${INSTALL_EXEC} $$i ${DLIBDIR}; done

clean:
	-$(RM) ${PROGS} ${OBJECTS} *~
