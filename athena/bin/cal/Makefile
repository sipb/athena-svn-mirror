#################################################################
#
#	Makefile for the entire Whatsup Calendar Package
#
#	Noah Mendelsohn
#	Created: 8/26/87
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/Makefile,v $
#	$Author: shanzer $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/Makefile,v 1.2 1987-10-29 12:20:09 shanzer Exp $
#
#################################################################

DESTDIR=
GDBINCLUDES= /usr/include
GDBLIBDIR= /usr/athena/lib

WHATSUPINCLUDES=../include
CFLAGS=	 "-O -I${WHATSUPINCLUDES} -I${GDBINCLUDES} -L${GDBLIBDIR}"
LINTFLAGS= "-I${WHATSUPINCLUDES} -I${GDBINCLUDES}"


##  USE THE FOLLOWING ON THE RT/PC WHERE -L IS NOT SUPPORTED
##  A CORREESPONDING CHANGE MUST BE MADE IN whatsup/Makefile
##  AND postit/Makefile.
##
##  All of this becomes irrelavent when and if libgdb.a is put
##  into /usr/lib where it belongs
# CFLAGS=	 "-O -I${WHATSUPINCLUDES} -I${GDBINCLUDES}"


SUBDIR= util whatsup postit include doc misc


all:	${SUBDIR} 

${SUBDIR}: FRC
	(cd $@; make ${MFLAGS} OCFLAGS=${CFLAGS} all)

FRC:

install: ${SUBDIR} 
	for i in ${SUBDIR} ; do\
	    (cd $$i; make ${MFLAGS} DESTDIR=${DESTDIR} install); done
	echo "Whatsup Calendar Package Installation Attempt Complete"

clean:
	rm -f a.out core *.s *.o *~ .*~
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} clean; cd ..); done

depend:
	for i in ${SUBDIR}; do \
	    (cd $$i; make ${MFLAGS} OCFLAGS=${CFLAGS} depend;cd ..); done
	echo "Whatsup Package  Makefile dependencies have been rebuilt"


lint:
	for i in ${SUBDIR} ; do \
	    (cd $$i; make ${MFLAGS} OLINTFLAGS=${LINTFLAGS} lint;cd ..); done


# DO NOT DELETE THIS LINE -- make depend may use it at some future time
# DEPENDENCIES MUST END AT END OF FILE
