###########################################################################
#
#				Makefile
#
#
#	Makefile for the Postit and Bulkpost Calendar Update Programs
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/Makefile,v $
#	$Author: epeisach $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/Makefile,v 1.5 1990-02-09 09:27:38 epeisach Exp $
#
###########################################################################

INSTDIR= /usr/athena

POSTSRC=   postmaster.c
POSTOBJ=   postmaster.o
BULKSRC=   bulkmaster.c
BULKOBJ=   bulkmaster.o
SHAREDSRCS=	postprint.c postdata.c postparse.c postprompts.c postmisc.c 
SHAREDOBJS=	postprint.o postdata.o postparse.o postprompts.o postmisc.o 
ALLSRCS=   ${POSTSRC} ${BULKSRC} ${SHAREDSRCS}
ALLOBJS=   ${POSTOBJ} ${BULKOBJ} ${SHAREDOBJS}

OTHERSRC=  ../util/what_lib.c
OTHEROBJ=  ../util/what_lib.o

INCLIB= ../include

CFLAGS=  ${OCFLAGS} 
LINTFLAGS= ${OLINTFLAGS}

GDBLIB= -lgdb
##
## Use the following for the RT/PC where -L does not work
##
# GDBLIB= ../../gdb_distribution/lib/libgdb.a

HESLIB= -lhesiod

.c.o:
	-${CC} -c ${CFLAGS} $*.c


all:	postit bulkpost

postit: ${POSTOBJ} ${SHAREDOBJS} ${OTHEROBJ}
	cc -o postit ${CFLAGS} ${POSTOBJ} ${SHAREDOBJS} ${OTHEROBJ} ${GDBLIB} \
	             ${HESLIB}

bulkpost: ${BULKOBJ} ${SHAREDOBJS} ${OTHEROBJ}
	cc -o bulkpost ${CFLAGS} ${BULKOBJ} ${SHAREDOBJS} ${OTHEROBJ} \
		       ${GDBLIB} ${HESLIB}

lint:
	lint -huv ${LINTFLAGS} ${POSTSRC} ${SHAREDSRCS} ${OTHERSRC} | \
		egrep -v "possible pointer alignment problem"
	lint -huv ${LINTFLAGS} ${BULKSRC} ${SHAREDSRCS} ${OTHERSRC} | \
		egrep -v "possible pointer alignment problem"

clean:
	rm -f *~ *.o core makedep eddep bulkpost postit TAGS

install: postit bulkpost
	install -c -s -m 755 postit ${DESTDIR}${INSTDIR}/postit
	install -c -s -m 755 bulkpost ${DESTDIR}${INSTDIR}/bulkpost

depend: 
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} -I${INCLIB} ${ALLSRCS}

TAGS:
	etags *.c ../util/*.c

