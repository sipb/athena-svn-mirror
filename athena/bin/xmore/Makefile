#      This is the Makefile for Xmore, a file browsing utility
#      built upon Xlib and the XToolkit.
#      
#      Created: 	October 22, 1987
#      By:		Chris D. Peterson
#     
#      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v $
#      $Author: shanzer $
#      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v 1.1 1988-05-03 17:32:28 shanzer Exp $
#      
#         Copyright 1987, 1988 by the Massachusetts Institute of Technology.
#     
#      For further information on copyright and distribution 
#      see the file mit-copyright.h

# Directories to put the things into on install.

DESTDIR= 

#The directory for the executables.

INSTDIR= /mit/kit/Binvax

#The directory to put the help file into.	

LIBDIR=/mit/kit/Lib

#The directory to put the manual pages into

MANDIR=/mit/kit/man
#The specific section to put xmore's manual page into, can be one of:
# 0 - 8, n or l (small L).
MANSECT=n

CFLAGS= -g -DATHENA -DHELPFILE=\"${LIBDIR}/xmore.help\" ${INCFLAGS}
INCFLAGS= -I/mit/X11/VAX
LDFLAGS= -L/mit/X11/VAX/lib/Xt -L/mit/X11/VAX/lib/Xaw -L/mit/X11/VAX/lib/X
LIBES= -lXaw -lXt -lX
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
	 strip xmore
	 cp xmore ${DESTDIR}${INSTDIR}
	 cp xmore.help ${LIBDIR}
	 cp xmore.man ${MANDIR}/man${MANSECT}/xmore.${MANSECT}
	 cp xmore.cat ${MANDIR}/cat${MANSECT}/xmore.${MANSECT}	

clean: 	;
	rm *~ *.o xmore xmore.shar core xmore.cat

saber: ;
	saber ${INCFLAGS} ${LDFLAGS} ${CFILES} ${LIBES}

shar: 	;
	shar README Makefile xmore.help xmore.man \
	     ${CFILES} ${XTKFILES} > xmore.shar

manual:	;
	nroff -man xmore.man > xmore.cat

