###########################################################################
# SCCS_data:	%Z% %M% %I% %E% %U%
#
# The other half of the things you need to tailor to port Wcl to a new
# platform # when you do not have Imake.  WcMake1.tmpl has the first half.
#
# This file gets included by the subdirectory Makefiles of programs
# (Ari, Mri, and Ori) after they set the following:
# CLIENT_INCS CLIENT_LIBS SYS_LIBRARIES
#
# This file gets included by the subdirectory Makefiles of libraries
# (Wc, Xp, Xop, and Xmp) after they set the following:
# LIBRARY_NAME OBJS
#

  WCL_VER = 2.7

###########################################################################
# Wcl libraries, and everything after client libraries
#
# WCL_LIBS = $(WCLIB) $(XMULIB) $(XTOOLLIB) $(XLIB)
  WCL_LIBS = $(WCLIB) $(XTOOLLIB) $(XLIB)

  LAST_LIBS = -lsocket
# LAST_LIBS = -lsocket -lnsl -lc -L/usr/ucblib -lucb

LOCAL_LIBRARIES = $(CLIENT_LIBS) $(WCL_LIBS) $(SYS_LIBRARIES) $(LAST_LIBS)
  LOCAL_LDFLAGS = $(CLIENT_LDFLAGS) $(WCLDFLAG) $(X_LIBS)

###########################################################################
# Library File Name
#
#LIBRARY_FILE = lib$(LIBRARY_NAME).so.$(WCL_VER)
#LIBRARY_FILE = lib$(LIBRARY_NAME).so
 LIBRARY_FILE = lib$(LIBRARY_NAME).a

###########################################################################
# This will be used for all compilations from .c.o for libraries
#
    ALL_INCS = -I.. $(CLIENT_INCS) -I$(INCROOT)

#CFLAGS_LIBS = -O  $(ALL_INCS) -DSUNSHLIB -DSHAREDCODE -pic
#CFLAGS_LIBS = -O  $(ALL_INCS)
#CFLAGS_LIBS = -O  $(ALL_INCS) -KPIC -Xc -DNARROWPROTO
#CFLAGS_LIBS = -O  $(ALL_INCS) -KPIC -Xc -DNARROWPROTO -DSYSV -DSVR4 -DSNI
 CFLAGS_LIBS = -Ox $(ALL_INCS)
#CFLAGS_LIBS = -g  $(ALL_INCS) 
#CFLAGS_LIBS = -g  $(ALL_INCS) -DXTTRACEMEMORY -DASSERTIONS

###########################################################################
# This will be used for all compilations from .c.o for applications
#
#CFLAGS_PROG = -O  -pipe $(ALL_INCS) -target sun4
#CFLAGS_PROG = -O  $(ALL_INCS)
#CFLAGS_PROG = -O  $(ALL_INCS) -Xc -DNARROWPROTO
#CFLAGS_PROG = -O  $(ALL_INCS) -Xc -DNARROWPROTO -DSYSV -DSVR4 -DSNI
 CFLAGS_PROG = -Ox $(ALL_INCS)

###########################################################################
# This will be used for all compilations from .c.o for test applications
#
#CFLAGS_TEST = -g  -pipe $(ALL_INCS)
#CFLAGS_TEST = -g  -pipe $(ALL_INCS) -DXTTRACEMEMORY -DASSERTIONS
#CFLAGS_TEST = -g  $(ALL_INCS) -Xc -DNARROWPROTO -DSYSV -DSVR4 -DSNI
 CFLAGS_TEST = -g  $(ALL_INCS)

###########################################################################
# Command To Build Libraries (and run ranlib if necessary)
#
#MKLIB_CMD = ld -o $(LIBRARY_FILE) -assert pure-text $(OBJS)
#MKLIB_CMD = rm -f $(LIBRARY_FILE).$(WCL_VER) ; $(LD) -o $(LIBRARY_FILE).$(WCL_VER) -G -z text -h $(LIBRARY_FILE).$(WCL_VER) $(OBJS) ; rm -f $(LIBRARY_FILE) ; ln -s $(LIBRARY_FILE).$(WCL_VER) $(LIBRARY_FILE)
#MKLIB_CMD = ar clq $(LIBRARY_FILE) $(OBJS) ; ranlib $(LIBRARY_FILE)
 MKLIB_CMD = ar clq $(LIBRARY_FILE) $(OBJS)

###########################################################################
# Command To Install Libraries
#
# CP_LIB = mv
# CP_LIB = rm -f $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE).$(WCL_VER) $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE) ; mv $(LIBRARY_FILE).$(WCL_VER) $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE).$(WCL_VER) ; ln -s $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE).$(WCL_VER) $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE) ; echo moved
# CP_LIB = cp -p
  CP_LIB = rm -f $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE) ; ar clq $(DESTDIR)$(USRLIBDIR)/$(LIBRARY_FILE) $(OBJS) ; echo moved

###########################################################################
# Command to update libraries after installation
#
# RANLIB = ranlib
  RANLIB = echo Done with


###########################################################################
# This will be used to link applications
#
#LDFLAGS_PROG = -O -pipe $(LOCAL_LDFLAGS) $(LOCAL_LIBRARIES)
 LDFLAGS_PROG = $(LOCAL_LDFLAGS) $(LOCAL_LIBRARIES)

###########################################################################
###########################################################################
###########################################################################
# You probably never need to edit anything below here
#
  SHELL = /bin/sh
   MAKE = make -f MakeByHand
