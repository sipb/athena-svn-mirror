# transcript/src/milan/Makefile
#
# Copyright (C) 1992 Adobe Systems Incorporated. All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release
# directory. 

# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/milan/Makefile,v 1.1.1.1 1996-10-07 20:25:53 ghudson Exp $

CC = $$CCOMP

CFLAGS = $$CCFLAGS

FILTEROBJECTS= fpfilter.o printer.o errors.o options.o system.o \
udpstat.o
STATUSOBJECTS=fpstatus.o udpstat.o

all: fpcomm fpstatus

fpcomm: ${FILTEROBJECTS} errors.h
	${CC} ${CFLAGS} -o fpcomm ${FILTEROBJECTS} ${SOCKFLAGS} ${RESOLVLIB}

fpstatus: ${STATUSOBJECTS} udp.h dp.h
	${CC} ${CFLAGS} -o fpstatus ${STATUSOBJECTS} ${SOCKFLAGS} ${RESOLVLIB}

