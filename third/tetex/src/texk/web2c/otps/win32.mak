################################################################################
#
# Makefile  : Web2C / Omega / otps
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/06/10 01:45:30 popineau>
#
################################################################################
root_srcdir = ..\..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

USE_KPATHSEA = 1
USE_GNUW32 = 1

# These get expanded from the parent directory, not this one.
!include <msvc/common.mak>

DEFS = -I.. $(DEFS) -DHAVE_CONFIG_H

LEX_OUTPUT_ROOT = lex_yy

proglib = ..\lib\$(objdir)\lib.lib
programs = $(objdir)\otp2ocp.exe $(objdir)\outocp.exe

otp2ocp_objects = $(objdir)\otp2ocp.obj $(objdir)\routines.obj              \
	$(objdir)\y_tab.obj $(objdir)\$(LEX_OUTPUT_ROOT).obj

default: all

all: $(objdir) $(programs)

$(objdir)\otp2ocp.exe: $(otp2ocp_objects) $(kpathsealib) # $(proglib)
	$(link) $(otp2ocp_objects) $(kpathsealib) $(conlibs)

$(objdir)\otp2ocp.obj: otp2ocp.c y_tab.h

y_tab.c y_tab.h: otp.y
	$(yacc) -d -v otp.y -o y_tab.c

$(objdir)\$(LEX_OUTPUT_ROOT).obj: otp.h

$(LEX_OUTPUT_ROOT).c: otp.l
	$(lex) -t otp.l | sed "/^extern int isatty YY/d" > $(LEX_OUTPUT_ROOT).c

$(objdir)\outocp.exe: $(objdir)\outocp.obj $(kpathsealib) # $(proglib)
	$(link) $(objdir)\outocp.obj $(kpathsealib) $(conlibs)

!include <msvc/config.mak>
!include <msvc/install.mak>

install:: install-exec

!include <msvc/clean.mak>

clean::
	-@$(del) y_tab.c y_tab.h y.output yacc.* $(LEX_OUTPUT_ROOT).c

!include <msvc/rdepend.mak>
!include "./depend.mak"

#
# Local Variables:
# mode: makefile
# End:
