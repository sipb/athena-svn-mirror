#      This is the Makefile for Xmore, a file browsing utility
#      built upon Xlib and the XToolkit.
#      
#      Created: 	October 22, 1987
#      By:		Chris D. Peterson
#     
#      $Source: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v $
#      $Author: epeisach $
#      $Header: /afs/dev.mit.edu/source/repository/athena/bin/xmore/Makefile,v 1.8 1990-02-09 08:59:58 epeisach Exp $
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

CFLAGS= -g -DATHENA -DHELPFILE=\"${LIBDIR}/xmore.help\" ${INCFLAGS}
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

# DO NOT DELETE THIS LINE -- make depend depends on it.

globals.o: globals.c more.h /usr/include/stdio.h /usr/include/X11/Xos.h
globals.o: /usr/include/sys/types.h /usr/include/strings.h
globals.o: /usr/include/sys/file.h /usr/include/sys/time.h
globals.o: /usr/include/sys/time.h /usr/include/sys/dir.h
globals.o: /usr/include/sys/stat.h /usr/include/X11/X.h
globals.o: /usr/include/X11/Xatom.h /usr/include/X11/StringDefs.h
globals.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
globals.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
globals.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
globals.o: /usr/include/X11/Constraint.h /usr/include/X11/AsciiText.h
globals.o: /usr/include/X11/copyright.h /usr/include/X11/Text.h
globals.o: /usr/include/X11/Command.h /usr/include/X11/Label.h
globals.o: /usr/include/X11/Simple.h /usr/include/X11/Xmu.h
globals.o: /usr/include/X11/Form.h /usr/include/X11/Constraint.h
globals.o: /usr/include/X11/Shell.h /usr/include/X11/VPaned.h
globals.o: /usr/include/X11/Paned.h ScrollByLine.h mit-copyright.h defs.h
help.o: help.c globals.h more.h /usr/include/stdio.h /usr/include/X11/Xos.h
help.o: /usr/include/sys/types.h /usr/include/strings.h
help.o: /usr/include/sys/file.h /usr/include/sys/time.h
help.o: /usr/include/sys/time.h /usr/include/sys/dir.h
help.o: /usr/include/sys/stat.h /usr/include/X11/X.h /usr/include/X11/Xatom.h
help.o: /usr/include/X11/StringDefs.h /usr/include/X11/Intrinsic.h
help.o: /usr/include/X11/Xlib.h /usr/include/X11/Xutil.h
help.o: /usr/include/X11/Xresource.h /usr/include/X11/Core.h
help.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
help.o: /usr/include/X11/AsciiText.h /usr/include/X11/copyright.h
help.o: /usr/include/X11/Text.h /usr/include/X11/Command.h
help.o: /usr/include/X11/Label.h /usr/include/X11/Simple.h
help.o: /usr/include/X11/Xmu.h /usr/include/X11/Form.h
help.o: /usr/include/X11/Constraint.h /usr/include/X11/Shell.h
help.o: /usr/include/X11/VPaned.h /usr/include/X11/Paned.h ScrollByLine.h
help.o: mit-copyright.h defs.h
main.o: main.c globals.h more.h /usr/include/stdio.h /usr/include/X11/Xos.h
main.o: /usr/include/sys/types.h /usr/include/strings.h
main.o: /usr/include/sys/file.h /usr/include/sys/time.h
main.o: /usr/include/sys/time.h /usr/include/sys/dir.h
main.o: /usr/include/sys/stat.h /usr/include/X11/X.h /usr/include/X11/Xatom.h
main.o: /usr/include/X11/StringDefs.h /usr/include/X11/Intrinsic.h
main.o: /usr/include/X11/Xlib.h /usr/include/X11/Xutil.h
main.o: /usr/include/X11/Xresource.h /usr/include/X11/Core.h
main.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
main.o: /usr/include/X11/AsciiText.h /usr/include/X11/copyright.h
main.o: /usr/include/X11/Text.h /usr/include/X11/Command.h
main.o: /usr/include/X11/Label.h /usr/include/X11/Simple.h
main.o: /usr/include/X11/Xmu.h /usr/include/X11/Form.h
main.o: /usr/include/X11/Constraint.h /usr/include/X11/Shell.h
main.o: /usr/include/X11/VPaned.h /usr/include/X11/Paned.h ScrollByLine.h
main.o: mit-copyright.h defs.h
pages.o: pages.c globals.h more.h /usr/include/stdio.h /usr/include/X11/Xos.h
pages.o: /usr/include/sys/types.h /usr/include/strings.h
pages.o: /usr/include/sys/file.h /usr/include/sys/time.h
pages.o: /usr/include/sys/time.h /usr/include/sys/dir.h
pages.o: /usr/include/sys/stat.h /usr/include/X11/X.h
pages.o: /usr/include/X11/Xatom.h /usr/include/X11/StringDefs.h
pages.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
pages.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
pages.o: /usr/include/X11/Core.h /usr/include/X11/Composite.h
pages.o: /usr/include/X11/Constraint.h /usr/include/X11/AsciiText.h
pages.o: /usr/include/X11/copyright.h /usr/include/X11/Text.h
pages.o: /usr/include/X11/Command.h /usr/include/X11/Label.h
pages.o: /usr/include/X11/Simple.h /usr/include/X11/Xmu.h
pages.o: /usr/include/X11/Form.h /usr/include/X11/Constraint.h
pages.o: /usr/include/X11/Shell.h /usr/include/X11/VPaned.h
pages.o: /usr/include/X11/Paned.h ScrollByLine.h mit-copyright.h defs.h
ScrollByLine.o: ScrollByLine.c /usr/include/X11/IntrinsicP.h
ScrollByLine.o: /usr/include/X11/Intrinsic.h /usr/include/X11/Xlib.h
ScrollByLine.o: /usr/include/sys/types.h /usr/include/X11/X.h
ScrollByLine.o: /usr/include/X11/Xutil.h /usr/include/X11/Xresource.h
ScrollByLine.o: /usr/include/X11/Xos.h /usr/include/strings.h
ScrollByLine.o: /usr/include/sys/file.h /usr/include/sys/time.h
ScrollByLine.o: /usr/include/sys/time.h /usr/include/X11/Core.h
ScrollByLine.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
ScrollByLine.o: /usr/include/X11/CoreP.h /usr/include/X11/CompositeP.h
ScrollByLine.o: /usr/include/X11/ConstrainP.h ScrollByLine.h ScrollByLineP.h
ScrollByLine.o: /usr/include/X11/Scroll.h /usr/include/X11/Xmu.h
ScrollByLine.o: /usr/include/X11/Intrinsic.h /usr/include/X11/StringDefs.h
ScrollByLine.o: X11/Misc.h
