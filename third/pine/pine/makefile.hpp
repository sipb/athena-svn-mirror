# $Id: makefile.hpp,v 1.1.1.1 2001-02-19 07:05:11 ghudson Exp $
#
#            T H E    P I N E    M A I L   S Y S T E M
#
#   Laurence Lundblade and Mike Seibel
#   Networks and Distributed Computing
#   Computing and Communications
#   University of Washington
#   Administration Building, AG-44
#   Seattle, Washington, 98195, USA
#   Internet: lgl@CAC.Washington.EDU
#             mikes@CAC.Washington.EDU
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
#   Pine is in part based on The Elm Mail System:
#    ***********************************************************************
#    *  The Elm Mail System  -  Revision: 2.13                             *
#    *                                                                     *
#    * 			Copyright (c) 1986, 1987 Dave Taylor               *
#    * 			Copyright (c) 1988, 1989 USENET Community Trust    *
#    ***********************************************************************
#
#


#
#     Make file for the Pine mail system for HP/UX.
#
#   Most commonly fiddled flags for compiler.
#   Uncomment the setttings desired here
#
RM=          rm -f
LN=          ln -s
MAKE=        make
OPTIMIZE=    # -O
PROFILE=     # -pg
DEBUG=       -g -DDEBUG

CCLIENTDIR=  ../c-client
PICODIR=     ../pico

# Only need to uncomment next two lines if you run make from this directory
# and you don't want to supply them as arguments to the make.
#LDAPLIBS=    ../ldap/libraries/libldap.a ../ldap/libraries/liblber.a
#LDAPCFLAGS=  -DENABLE_LDAP -I../ldap/include
# Object files that need to be rebuilt if ENABLE_LDAP gets defined.
LDAPOFILES=   addrbook.o adrbkcmd.o args.o bldaddr.o init.o \
	      mailview.o other.o pine.o strings.o takeaddr.o

STDLIBS=     -ltermcap
LOCLIBS=     $(PICODIR)/libpico.a $(CCLIENTDIR)/c-client.a
LIBS=        $(EXTRALIBES) $(LOCLIBS) $(LDAPLIBS) $(STDLIBS) \
             `cat $(CCLIENTDIR)/LDFLAGS`

STDCFLAGS=   -Dconst= -Aa -D_HPUX_SOURCE -DHPP -DSYSTYPE=\"HPP\" -DMOUSE
CFLAGS=      $(OPTIMIZE) $(PROFILE) $(DEBUG) $(EXTRACFLAGS) $(LDAPCFLAGS) \
	     $(STDCFLAGS)

OFILES=	addrbook.o adrbkcmd.o adrbklib.o args.o bldaddr.o context.o filter.o \
	folder.o help.o helptext.o imap.o init.o mailcap.o mailcmd.o \
	mailindx.o mailpart.o mailview.o newmail.o other.o pine.o \
	reply.o screen.o send.o signals.o status.o strings.o takeaddr.o \
	os.o

HFILES=	headers.h os.h pine.h context.h helptext.h \
	$(PICODIR)/headers.h $(PICODIR)/estruct.h \
	$(PICODIR)/edef.h $(PICODIR)/efunc.h \
	$(PICODIR)/pico.h $(PICODIR)/os.h \
	$(CCLIENTDIR)/mail.h $(CCLIENTDIR)/osdep.h \
	$(CCLIENTDIR)/rfc822.h $(CCLIENTDIR)/misc.h

# The & means make in parallel
pine:  $& $(OFILES) $(LOCLIBS)
	echo "char datestamp[]="\"`date`\"";" > date.c
	echo "char hoststamp[]="\"`hostname`\"";" >> date.c
	$(PURIFY) $(PURIFYOPTIONS) $(CC) $(LDFLAGS) $(CFLAGS) -o pine $(OFILES) date.c $(LIBS)

abookcpy:	abookcpy.o $(LOCLIBES)
	$(CC) $(LDFLAGS) $(CFLAGS) -o abookcpy abookcpy.o $(LIBS)

pine-use:	pine-use.c
	$(CC) -o pine-use pine-use.c

clean:
	$(RM) *.o os.h os.c helptext.c helptext.h pine
	cd osdep; make clean; cd ..

os.h:	osdep/os-hpp.h
	$(RM) os.h
	$(LN) osdep/os-hpp.h os.h

os.c:	osdep/os-hpp.c
	$(RM) os.c
	$(LN) osdep/os-hpp.c os.c

$(OFILES):						$(HFILES)
addrbook.o adrbkcmd.o adrbklib.o bldaddr.o takeaddr.o:	adrbklib.h
context.o:						$(CCLIENTDIR)/misc.h
send.o:							$(CCLIENTDIR)/smtp.h
#$(LDAPOFILES):						$(LDAPLIBS)

helptext.c:	pine.hlp
		./cmplhelp.sh  < pine.hlp > helptext.c

helptext.h:	pine.hlp
		./cmplhlp2.sh  < pine.hlp > helptext.h

osdep/os-hpp.c:	osdep/bld_path osdep/canacces osdep/canonicl \
		osdep/chnge_pw osdep/coredump osdep/creatdir \
		osdep/diskquot.hpp osdep/domnames osdep/err_desc \
		osdep/expnfldr osdep/fgetpos osdep/filesize osdep/fltrname \
		osdep/fnexpand osdep/header osdep/hostname.una \
		osdep/jobcntrl osdep/lstcmpnt osdep/mimedisp osdep/pipe \
		osdep/print osdep/pw_stuff osdep/readfile osdep/debuging.tim \
		osdep/rename osdep/tempfile osdep/writ_dir \
		osdep/termin.unx osdep/termout.unx \
		osdep/termin.gen osdep/termout.gen \
		osdep/sendmail osdep/execview osdep/srandom.dum \
		osdep/sunquota osdep/postreap.wtp osdep/os-hpp.ic
		cd osdep; $(MAKE) includer os-hpp.c; cd ..
