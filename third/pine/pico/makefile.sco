# $Id: makefile.sco,v 1.1.1.2 2003-02-12 07:58:32 ghudson Exp $
#
#   Michael Seibel
#   Networks and Distributed Computing
#   Computing and Communications
#   University of Washington
#   Administration Builiding, AG-44
#   Seattle, Washington, 98195, USA
#   Internet: mikes@cac.washington.edu
#
#   Please address all bugs and comments to "pine-bugs@cac.washington.edu"
#
#
#   Pine and Pico are registered trademarks of the University of Washington.
#   No commercial use of these trademarks may be made without prior written
#   permission of the University of Washington.
#
#   Pine, Pico, and Pilot software and its included text are Copyright
#   1989-1998 by the University of Washington.
#
#   The full text of our legal notices is contained in the file called
#   CPYRIGHT, included with this distribution.
#

#
# Makefile for SCO 3.2vx version of the PINE composer library and 
# stand-alone editor pico.
#
# Port courtesy of Robert Lewis <robertle@sco.COM>, 921217
#

RM=          rm -f
LN=          ln -s
MAKE=        make
OPTIMIZE=    # -O
PROFILE=     # -pg
DEBUG=       -g -DDEBUG

STDCFLAGS=	-Dsco -DJOB_CONTROL -DPOSIX -DMOUSE
CFLAGS=         $(OPTIMIZE) $(PROFILE) $(DEBUG) $(EXTRACFLAGS) $(STDCFLAGS)

# switches for library building
LIBCMD=		ar
LIBARGS=	ru
RANLIB=		true

# When USE_TERMINFO is defined link with library tinfo.
LIBS=		$(EXTRALDFLAGS) -lc -lc_s -lx -lm -ltinfo
# When USE_TERMCAP is defined link with library termcap.
#LIBS=		$(EXTRALIBES) -lc -lc_s -lx -lm -ltermcap

OFILES=		attach.o basic.o bind.o browse.o buffer.o \
		composer.o display.o file.o fileio.o line.o pico_os.o \
		pico.o random.o region.o search.o \
		window.o word.o

HFILES=		headers.h estruct.h edef.h efunc.h pico.h os.h

#
# dependencies for the Unix versions of pico and libpico.a
#
all:		pico pilot
pico pilot:	libpico.a

pico:		main.o
		$(CC) $(CFLAGS) main.o libpico.a $(LIBS) -o pico

pilot:		pilot.o
		$(CC) $(CFLAGS) pilot.o libpico.a $(LIBS) -o pilot

libpico.a:	$(OFILES)
		$(LIBCMD) $(LIBARGS) libpico.a $(OFILES)
		$(RANLIB) libpico.a

clean:
		rm -f *.a *.o *~ pico_os.c os.h pico pilot
		cd osdep; $(MAKE) clean; cd ..

os.h:		osdep/os-sco.h
		$(RM) os.h
		$(LN) osdep/os-sco.h os.h

pico_os.c:	osdep/os-sco.c
		$(RM) pico_os.c
		$(LN) osdep/os-sco.c pico_os.c

$(OFILES) main.o pilot.o:	$(HFILES)
pico.o:				ebind.h

osdep/os-sco.c:	osdep/header osdep/unix osdep/read.sel osdep/raw.io \
		osdep/spell.unx osdep/term.inf osdep/truncate osdep/fsync.non \
		osdep/os-sco.ic
		cd osdep; $(MAKE) includer os-sco.c; cd ..