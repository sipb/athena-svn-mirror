# transcript/src/printpanel/Makefile
#
# Copyright (C) 1992 Adobe Systems Incorporated. All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/printpanel/Makefile,v 1.1.1.1 1996-10-07 20:25:54 ghudson Exp $

SHELL = /bin/sh

CC = $$CCOMP

CFLAGS= $$CCFLAGS -I..

LFLAGS= $$LDFLAGS

FILES = PrintPanel.o ../psutil.o readppd.o ppanel.o readfax.o

HEADERS = PrintPanel.h PrintPaneP.h ../config.h ../transcript.h 

all: ppanel

ppanel: ${HEADERS} ${FILES}
	${CC} ${CFLAGS} -o ppanel ${LFLAGS} ${FILES} -lXm -lXt -lX11 \
	$$SPECIALLIBS 

readppd.o readfax.o: PrintPanel.c

#lint:
#	lint -v -I/usr/local/X11R4/include PrintPanel.c readppd.c \
#psutil.c ppanel.c readfax.c
