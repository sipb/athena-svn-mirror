###########################################################################
# SCCS_data: 	%Z% %M% %I% %E% %U%
#
# Half of the things you need to tailor to port Wcl to a new platform
# when you do not have Imake.  WcMake2.tmpl has the other half.
#

###########################################################################
# Hacks for various release levels of your X and Xt system:
# If X11R3.5 (i.e., Motif 1.0 Xt) these need to be uncommented:
      OLD_XT_SRCS = Xt4GetResL.c XtMacros.c XtName.c
      OLD_XT_OBJS = Xt4GetResL.o XtMacros.o XtName.o
        BROKEN_XT = -DXtNameToWidgetBarfsOnGadgets

###########################################################################
# If early X11R4 where XtNameToWidget barfs on Gadgets, uncomment these
#     OLD_XT_SRCS = XtName.c
#     OLD_XT_OBJS = XtName.o
#       BROKEN_XT = -DXtNameToWidgetBarfsOnGadgets

###########################################################################
# Comment out any widget sets you do not have.
#
#   WcATHENA = Xp Ari
     WcMOTIF = Xmp Mri
#WcOPEN_LOOK = Xop Ori

###########################################################################
# Special Flags for compiling files which use widget libraries
#
   XP_LIB_OPTS =
  XMP_LIB_OPTS = -D_OLD_MOTIF
# XMP_LIB_OPTS = -D_NO_PROTO
  XOP_LIB_OPTS = -I/usr/openwin/include -I/usr/openwin/include/Xol

###########################################################################
# The location of your X include files and libraries, and where Wcl
# libraries and programs will be installed.
#
# Typical R5 locations:
#     INCROOT = /usr/X11R5/include
#      INCDIR = $(INCROOT)/X11
#   USRLIBDIR = /usr/X11R5/lib
#      LIBDIR = $(USRLIBDIR)/X11
# XAPPLOADDIR = $(LIBDIR)/app-defaults
#      BINDIR = /usr/X11R5/bin
#      X_LIBS = -L/usr/X11R5/lib
#
# Typical R4 and R3 locations:
      INCROOT = /usr/include
       INCDIR = $(INCROOT)/X11
    USRLIBDIR = /usr/lib
       LIBDIR = $(USRLIBDIR)/X11
  XAPPLOADDIR = $(LIBDIR)/app-defaults
       BINDIR = /usr/bin
       X_LIBS =
#
# Typical SunOS locations:
#     INCROOT = /usr/openwin/include
#      INCDIR = $(INCROOT)/X11
#   USRLIBDIR = /usr/openwin/lib
#      LIBDIR = /usr/lib/X11
# XAPPLOADDIR = $(LIBDIR)/app-defaults
#      BINDIR = /usr/openwin/bin
#      X_LIBS = -L/usr/openwin/lib


###########################################################################
# If you are building for SunOS or SVR4, these must be uncommented
#
#        DYNLIB = -ldl
#DYN_LINK_FLAGS = -DWC_HAS_dlopen_AND_dlsym
#       WcSTUFF = Stuff

###########################################################################
# Names of libraries
#
# Widget Libraries
#
             XAWLIB = -lXaw
              XMLIB = -lXm
#             XMLIB = -lXm -lgen
#             XMLIB = -lXm -lPW
             XOLLIB = -lXol
#
# Wcl Distribution Libraries
#
              WCLIB = -lWc $(DYNLIB)
           WCLDFLAG = -L$(WCTOPDIR)/Wc
              XPLIB = -lXp
           XPLDFLAG = -L$(WCTOPDIR)/Xp
             XMPLIB = -lXmp
          XMPLDFLAG = -L$(WCTOPDIR)/Xmp
             XOPLIB = -lXop
          XOPLDFLAG = -L$(WCTOPDIR)/Xop
#
# X11 Libraries
#
             XMULIB = -lXmu
           XTOOLLIB = -lXt
#      EXTENSIONLIB = -lXext
               XLIB = $(EXTENSIONLIB) -lX11


###########################################################################
# Only a very poorly configured system (like SCO) needs this
#
  LOCAL_STRINGS_H = ./strings.h
# GET_LOCAL_STRINGS_H = ln /usr/include/string.h ./strings.h
  GET_LOCAL_STRINGS_H = cp /usr/include/string.h ./strings.h

###########################################################################
# If you are building for SunOS, uncomment this.  It really means that
# the printf() supports %digit$ for specifying argument number.
# WONDER_PRINTF = -DSUNOS_PRINTF

###########################################################################
# The C compiler to use
#
  CC = cc
# CC = LD_RUN_PATH=$(USRLIBDIR) ; export LD_RUN_PATH ; /usr/ccs/bin/cc
# CC = gcc -fstrength-reduce -fpcc-struct-return

###########################################################################
# The Loader to use
#
  LD = cc
# LD = LD_RUN_PATH=$(USRLIBDIR) ; export LD_RUN_PATH ; /usr/ccs/bin/cc

###########################################################################
# The C pre-processor to use for app-defaults files
#
  CPP = /lib/cpp

###########################################################################
# For putting Wcl subdir include files in ./X11/$(LIB) for easy access
#
# LN = ln -s
  LN = ln

###########################################################################
# The owner of installed programs, include files, and libraries
#
  OWNER = root

###########################################################################
# Access flags for installed files
#
     INSTDIRFLAGS = 0755
     INSTBINFLAGS = 0755
     INSTLIBFLAGS = 0644
     INSTINCFLAGS = 0444
     INSTMANFLAGS = 0444 
  INSTAPPDEFFLAGS = 0444

###########################################################################
# Man page locations and suffixes
#
        PGMMANDIR = /usr/man/cat.C
     PGMMANSUFFIX = C.z
  PGMMANSRCSUFFIX = C.z
        LIBMANDIR = /usr/man/cat.S
     LIBMANSUFFIX = S.z
  LIBMANSRCSUFFIX = S.z

