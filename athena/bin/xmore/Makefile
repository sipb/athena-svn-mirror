#      This is the Makefile for Xmore, a file browsing utility
#      built upon Xlib and the XToolkit.
#      
#      Created: 	October 22, 1987
#      By:		Chris D. Peterson
#     
#      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v $
#      $Author: epeisach $
#      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v 1.9 1990-05-01 14:48:35 epeisach Exp $
#      
#         Copyright 1987, 1988 by the Massachusetts Institute of Technology.
#     
#      For further information on copyright and distribution 
#      see the file mit-copyright.h

# Directories to put the things into on install.

DESTDIR= 
INSTDIR= /usr/athena
LIBDIR=/usr/athena/lib
MANDIR=/usr/man
MANSECT=1

CFLAGS= -g -DHELPFILE=\"${LIBDIR}/xmore.help\" ${INCFLAGS}
INCFLAGS=
LIBES= -lXaw -lXt -lXmu -lX11
OBJS= globals.o help.o main.o pages.o ${XTKOBJS}
XTKOBJS= ScrollByLine.o
XTKFILES= ScrollByLine.c
INCLUDE= defs.h globals.h more.h mit-copyright.h
CFILES= globals.c help.c main.c pages.c ${XTKFILES}

all:		xmore
xmore:		${OBJS}
		cc ${CFLAGS} ${LDFLAGS} ${OBJS} -o xmore ${LIBES}
globals.o:	globals.c ${INCLUDE}
help.o:		help.c ${INCLUDE}
main.o:		main.c ${INCLUDE}
pages.o:	pages.c ${INCLUDE}
ScrollByLine.c: ScrollByLine.c ScrollByLineP.h ScrollByLine.h

install: ;
	 install -s xmore ${DESTDIR}${INSTDIR}/
	 cp xmore.help ${DESTDIR}${LIBDIR}
	 cp xmore.man ${DESTDIR}${MANDIR}/man${MANSECT}/xmore.${MANSECT}

clean: 	;
	rm -f *~ *.o xmore xmore.shar core xmore.cat

saber: ;
	saber ${INCFLAGS} ${LDFLAGS} ${CFILES} ${LIBES}

shar: 	;
	shar README Makefile xmore.help xmore.man \
	     ${CFILES} ${XTKFILES} > xmore.shar

manual:	;
	nroff -man xmore.man > xmore.cat


depend:
	touch Make.depend; makedepend -fMake.depend ${CFLAGS} ${CFILES} 

