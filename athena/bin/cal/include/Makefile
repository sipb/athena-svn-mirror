###########################################################################
#
#				Makefile
#
#
#	Makefile for the Whatsup Calendar include files.
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/Makefile,v $
#	$Author: probe $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/Makefile,v 1.1 1993-10-12 05:35:22 probe Exp $
#
###########################################################################

DESTDIR=
INSTDIR=/include
INCLUDEFILES=whatsup.h postit.h mit-copyright.h
CFLAGS=	 
LINTFLAGS= 

SUBDIR= 

all:	

install:
#	for i in ${INCLUDEFILES}; do  \
#		(install -c $$i ${DESTDIR}${INSTDIR}/$$i); \
#	done

clean:
	rm -f a.out core *.s *.o *~
#	for i in ${SUBDIR} ; do (cd $$i; make ${MFLAGS} clean; cd ..); done

depend:
#	for i in ${SUBDIR}; do \
#	    (cd $$i; make ${MFLAGS} depend;cd ..); done

lint:
#	for i in ${SUBDIR}; do \
#	    (cd $$i; make ${MFLAGS} OLINTFLAGS=${LINTFLAGS} depend;cd ..); done


# DO NOT DELETE THIS LINE -- make depend may use it at some future time
# DEPENDENCIES MUST END AT END OF FILE
