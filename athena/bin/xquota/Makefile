#       This is the file Makefile for the Xquota. 
#       
#       Created: 	October 22, 1987
#       By:		Chris D. Peterson
#    
#       $Source: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v $
#       $Author: epeisach $
#       $Header: /afs/dev.mit.edu/source/repository/athena/bin/xquota/Makefile,v 1.10 1990-03-28 18:23:27 epeisach Exp $
#       
#          Copyright 1987, 1988 by the Massachusetts Institute of Technology.
#    
#       For further information on copyright and distribution 
#       see the file mit-copyright.h
    
LIBDIR= /usr/lib/X11/app-defaults

HELPFILES= -DDEFAULTS_FILE=\"${LIBDIR}/Xquota.ad\"\
	   -DTOP_HELP_FILE=\"${LIBDIR}/top_help.txt\"\
	   -DPOPUP_HELP_FILE=\"${LIBDIR}/pop_help.txt\"

DESTDIR=
CFLAGS= -O ${INCFLAGS} ${HELPFILES}
SFLAGS= ${INCFLAGS} 
INCFLAGS= -I.
LDFLAGS=
LIBS= -lXaw -lXt -lXmu -lX11 -lhesiod -lrpcsvc
SRCS=handler.c main.c quota.c widgets.c
OBJS=handler.o main.o quota.o widgets.o
INCLUDE= xquota.h
CFILES=handler.c main.c quota.c widgets.c
CC=cc		# hc1.4 can't deal with widget.c

all:		xquota
xquota:		${OBJS}
		cc ${CFLAGS} ${LDFLAGS} ${OBJS} -o xquota ${LIBS}
clean:
	rm -f *~ *.o xquota xquota.shar core xquota.cat 

install:
	install -c -s xquota ${DESTDIR}/usr/athena/xquota
	for i in Xquota.ad top_help.txt pop_help.txt; do \
		install -c -m 0444 $$i ${DESTDIR}${LIBDIR}/$$i; \
		done

depend:
	touch Make.depend; makedepend -fMake.depend  -s "# DO NOT DELETE THIS LINE -- make depend uses it"\
                $(CFLAGS) $(INCFLAGS) ${SRCS}

