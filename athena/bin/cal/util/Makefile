###########################################################################
#
#				Makefile
#
#
#	Makefile for the whatsup utility object files
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/Makefile,v $
#	$Author: epeisach $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/Makefile,v 1.2 1989-09-16 12:56:55 epeisach Exp $
#
#
###########################################################################

INSTDIR=

SRCS=	 what_lib.c
OBJS = what_lib.o

CFLAGS=  ${OCFLAGS} 
LINTFLAGS =${OLINTFLAGS} 

SUBDIR= 
SCRIPT= 
STD=	
NSTD=	

.c.o:
	${CC} -c ${CFLAGS} $*.c

all: ${OBJS}

install: 

clean:
	rm -f a.out core *.s *.o *~ .*~ makedep eddep

lint:
#		util files are linted with the application source

depend:
	mkdep ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- mkdep uses it.

what_lib.o: what_lib.c /usr/include/mit-copyright.h /usr/include/stdio.h
what_lib.o: /usr/include/curses.h /usr/include/sgtty.h
what_lib.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
what_lib.o: /usr/include/sys/ttydev.h /usr/include/gdb.h
what_lib.o: /usr/include/sys/types.h /usr/include/sys/time.h
what_lib.o: /usr/include/sys/time.h
