###########################################################################
#
#				Makefile
#
#
#	Makefile for the Whatsup Calendar miscellaneous files.
#
#	Copyright 1987 by the Massachusetts Institute of Technology.
#	For copying and distribution information, see the file
#	"mit-copyright.h".
#
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/misc/Makefile,v $
#	$Author: shanzer $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/misc/Makefile,v 1.2 1987-10-29 12:39:43 shanzer Exp $
#
###########################################################################

.SUFFIXES: .el .elc

FORMSINSTDIR=/usr/athena
MACROINSTDIR=/usr/athena

BLANKFORMS= bulkpost.longform bulkpost.shortform
EMACSMACROS= postit.el
COMPILEDMACROS= postit.elc


.el.elc:
	emacs -batch -f batch-byte-compile $*.el


SUBDIR= 

all:	${COMPILEDMACROS}

install: all
#	for i in ${SUBDIR}; do \
#	    (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	for i in ${BLANKFORMS}; do \
		(install -c $$i ${DESTDIR}${FORMSINSTDIR}/$$i); \
	done
	for i in ${EMACSMACROS} ${COMPILEDMACROS}; do \
		(install -c $$i ${DESTDIR}${MACROINSTDIR}/$$i); \
	done

clean:
	rm -f a.out core *.s *.o *~ *.PS *.err *.man *.otl *.aux *.elc
#	for i in ${SUBDIR} ; do (cd $$i; make ${MFLAGS} clean; cd ..); done

depend:
#	for i in ${SUBDIR}; do \
#	    (cd $$i; make ${MFLAGS} depend;cd ..); done

lint:
#	for i in ${SUBDIR}; do \
#	    (cd $$i; make ${MFLAGS} OLINTFLAGS=${LINTFLAGS} depend;cd ..); done


# DO NOT DELETE THIS LINE -- make depend may use it at some future time
# DEPENDENCIES MUST END AT END OF FILE


