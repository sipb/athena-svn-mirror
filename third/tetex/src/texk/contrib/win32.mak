################################################################################
#
# Makefile  : TeXk / contrib
# Author    : Fabrice Popineau <Fabrice.Popineau@supelec.fr>
# Platform  : Win32, Microsoft VC++ 6.0, depends upon fpTeX 0.5 sources
# Time-stamp: <02/09/23 17:40:11 popineau>
#
################################################################################
root_srcdir=..\..
INCLUDE=$(INCLUDE);$(root_srcdir)\texk

USE_KPATHSEA=1
USE_MKTEX=1
MAKE_MKTEX = 1
USE_REGEX=1
USE_GNUW32=1

!include <msvc/common.mak>

DEFS = $(DEFS) -DHAVE_CONFIG_H

mktex_objs = $(objdir)\fileutils.obj	\
	$(objdir)\mktex.obj		\
	$(objdir)\variables.obj 	\
	$(objdir)\stackenv.obj

mktex_progs = $(objdir)\mktexlsr.exe	\
	$(objdir)\mktexnam.exe		\
	$(objdir)\mktexupd.exe		\
	$(objdir)\mktexpk.exe		\
	$(objdir)\mktexdir.exe		\
	$(objdir)\mktexmf.exe		\
	$(objdir)\mktextfm.exe		\
#	$(objdir)\mkofm.exe		\
	$(objdir)\mktextex.exe

programs = \
	$(mktexdll)			\
	$(objdir)\dvihp.exe		\
	$(objdir)\makempx.exe		\
	$(objdir)\mktex.exe		\
	$(objdir)\fmtutil.exe		\
	$(objdir)\mkocp.exe		\
	$(objdir)\mkofm.exe


libfiles = $(mktexlib)

default: all

all: $(objdir) $(mktex) $(programs)

lib: $(objdir) $(mktexlib)

!ifdef MKTEX_DLL
DEFS = -DMAKE_MKTEX_DLL $(DEFS) 

$(mktexlib): $(mktex_objs) 
	$(archive) /DEF $(mktex_objs)

$(mktexdll): $(mktex_objs) $(objdir)\libmktex.res $(regexlib) $(kpathsealib)
	$(link_dll) $(**) $(mktexlib:.lib=.exp) $(conlibs)
!else
$(mktexlib): $(mktex_objs) $(regexlib)
	$(archive) $(**)
!endif

mktex_progs: $(objdir)\mktex.exe
	for %%i in ($(mktex_progs)) \
		do $(copy) $(objdir)\mktex.exe $(objdir)\%%~nxi

$(objdir)\makempx.exe: $(objdir)\makempx.obj $(mktexlib) $(objdir)\makempx.res $(kpathsealib)
	$(link) $(**) $(conlibs)

$(objdir)\fmtutil.exe: $(objdir)\fmtutil.obj $(mktexlib) $(kpathsealib)
	$(link) $(**) $(conlibs)

$(objdir)\dvihp.exe: $(objdir)\dvihp.obj $(mktexlib) $(objdir)\dvihp.res $(kpathsealib)
	$(link) $(**) $(conlibs)

$(objdir)\mktex.exe: $(objdir)\main.obj $(mktexlib) $(kpathsealib)
	$(link) $(**) $(conlibs)

$(objdir)\mkocp.exe: $(objdir)\mkocp.obj $(mktexlib) $(kpathsealib)
	$(link) $(**) $(conlibs)

$(objdir)\mkofm.exe: $(objdir)\mkofm.obj $(gnuw32lib)
	$(link) $(**) $(conlibs)

$(objdir)\makempx.obj:	makempx.c
	$(compile) -UMAKE_MKTEX_DLL makempx.c

$(objdir)\fmtutil.obj:	fmtutil.c
	$(compile) -UMAKE_MKTEX_DLL fmtutil.c

$(objdir)\main.obj: main.c
	$(compile) -UMAKE_MKTEX_DLL main.c

$(objdir)\dvihp.obj: dvihp.c
	$(compile) -UMAKE_MKTEX_DLL dvihp.c

$(objdir)\mkocp.obj: mkocp.c
	$(compile) -UMAKE_MKTEX_DLL mkocp.c

$(objdir)\mkofm.obj: mkofm.c
	$(compile) -UKPSE_DLL -UMAKE_MKTEX_DLL mkofm.c

!include <msvc/config.mak>
!include <msvc/install.mak>

install:: install-exec

install-exec::
	-@echo $(verbose) & ( \
		echo "Installing mktex program files in $(MAKEDIR)" & \
		( for %%i in ($(mktex_progs)) do \
			$(copy) $(objdir)\mktex.exe $(bindir)\%%~nxi $(redir_stdout) ) & \
		$(copy) $(objdir)\fmtutil.exe $(bindir)\mktexfmt.exe $(redir_stdout) \
	)

!include <msvc/clean.mak>

!include <msvc/rdepend.mak>
!include "./depend.mak"

#Local Variables:
#mode: Makefile
#End:
