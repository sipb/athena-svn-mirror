#       This is the file Makefile for the Xquota. 
#       
#       Created: 	October 22, 1987
#       By:		Chris D. Peterson
#    
#       $Source: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v $
#       $Author: kit $
#       $Header: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v 1.2 1989-03-28 17:14:24 kit Exp $
#       
#          Copyright 1987, 1988 by the Massachusetts Institute of Technology.
#    
#       For further information on copyright and distribution 
#       see the file mit-copyright.h
    
LIBDIR= /mit/StaffTools/lib/xquota

HELPFILES= -DDEFAULTS_FILE=\"${LIBDIR}/Xquota.ad\"\
	   -DTOP_HELP_FILE=\"${LIBDIR}/top_help.txt\"\
	   -DPOPUP_HELP_FILE=\"${LIBDIR}/pop_help.txt\"

CFLAGS= -O ${INCFLAGS} ${HELPFILES}
SFLAGS= ${INCFLAGS} 
INCFLAGS= -I. -I/u1/include
LDFLAGS= -L/u1/lib -L/usr/athena/lib
LIBS= -lXaw -lXt -lXmu -lX11 -lhesiod -lrpcsvc
OBJS=handler.o main.o quota.o widgets.o
INCLUDE= xquota.h
CFILES=handler.c main.c quota.c widgets.c

all:		xquota
xquota:		${OBJS}
		cc ${CFLAGS} ${LDFLAGS} ${OBJS} -o xquota ${LIBS}
clean: 	;
	rm -f *~ *.o xquota xquota.shar core xquota.cat 

# For use with Saber C.  If you don't have it, get it.

saber: ;
	/mit/kaufer/saber ${SFLAGS} ${LDFLAGS} ${CFILES} ${LIBS}

	 

