# transcript/src/Makefile.bsd
#
# Copyright (C) 1985,1987,1990,1991,1992 Adobe Systems Incorporated. All
# rights reserved. 
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/Makefile,v 1.1.1.1 1996-10-07 20:25:46 ghudson Exp $

SHELL = /bin/sh

CC = $$CCOMP

PROGS = psbanner pscomm psdman enscript ps4014 ps630 pscat pscatmap \
	psdit psplot pslpr psnup afmdit map asc85ec lzwec psfax psdraft

OBJECTS = psbanner.o pscomm.o psdman.o \
	enscript.o ps4014.o ps630.o pscat.o pscatmap.o \
	psdit.o psplot.o pslpr.o psnup.o \
	psutil.o psparse.o psfax.o psdraft.o

CFLAGS = $$CCFLAGS

programs: ${PROGS} 

afmdit: afmdit.o afmdit.h
	${CC} ${CFLAGS} -o afmdit afmdit.o

asc85ec: asc85ec.o
	${CC} ${CFLAGS} -o asc85ec asc85ec.o

lzwec: lzwec.o
	${CC} ${CFLAGS} -o lzwec lzwec.o

psnup: psnup.o psutil.o
	${CC} ${CFLAGS} -o psnup psutil.o  psnup.o -lm

pslpr: pslpr.o psparse.o psutil.o PSres.o compat.o
	${CC} ${CFLAGS} -o pslpr pslpr.o psparse.o psutil.o PSres.o compat.o

enscript: enscript.o psutil.o PSres.o compat.o
	${CC} ${CFLAGS} -o enscript enscript.o psutil.o PSres.o compat.o

pscat: pscat.o psutil.o 
	${CC} ${CFLAGS} -o pscat pscat.o psutil.o 

psbanner: psbanner.o psutil.o
	${CC} ${CFLAGS} -o psbanner psbanner.o psutil.o 

pscomm: pscomm.o psutil.o 
	${CC} ${CFLAGS} -o pscomm pscomm.o psutil.o 

psdman: psdman.o psutil.o  psparse.o PSres.o compat.o
	${CC} ${CFLAGS} -o psdman psdman.o psutil.o  psparse.o PSres.o compat.o
 
psplot: psplot.o psutil.o 
	${CC} ${CFLAGS} -o psplot psplot.o psutil.o

ps630: ps630.o psutil.o
	${CC} ${CFLAGS} -o ps630 ps630.o psutil.o

ps4014: ps4014.o psutil.o
	${CC} ${CFLAGS} -o ps4014 ps4014.o psutil.o -lm

pscatmap: pscatmap.o psutil.o PSres.o
	${CC} ${CFLAGS} -o pscatmap pscatmap.o psutil.o PSres.o

psdit: psdit.o psutil.o 
	${CC} ${CFLAGS} -o psdit psdit.o psutil.o  

map: map.o PSres.o
	${CC} ${CFLAGS} -o map map.o PSres.o

psfax: psfax.o
	${CC} ${CFLAGS} -o psfax psfax.o

psdraft: psdraft.o psutil.o
	${CC} ${CFLAGS} -o psdraft psdraft.o psutil.o


${OBJECTS}: transcript.h config.h compat.h
pscomm.o psbanner.o pscat.o: psspool.h
pscat.o pscatmap.o: action.h 
lzwec.o: lzw.h
pslpr.o psdman.o enscript.o PSres.o: PSres.h
compat.o: compat.c


